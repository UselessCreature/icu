/*
*******************************************************************************
*
*   Copyright (C) 1999-2006, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  gennames.c
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 1999nov01
*   created by: Markus W. Scherer
*
*   This program reads a binary file and creates a C source code file
*   with a byte array that contains the data of the binary file.
*
*   12/09/1999  weiv    Added multiple file handling
*/

#include "unicode/utypes.h"

#ifdef U_WINDOWS
#   define VC_EXTRALEAN
#   define WIN32_LEAN_AND_MEAN
#   define NOUSER
#   define NOSERVICE
#   define NOIME
#   define NOMCX
#include <windows.h>
#include <time.h>
#endif

#ifdef U_LINUX
#   define U_ELF
#endif

#ifdef U_ELF
#   include <elf.h>
#   if defined(ELFCLASS64)
#       define U_ELF64
#   endif
    /* Old elf.h headers may not have EM_X86_64, or have EM_X8664 instead. */
#   ifndef EM_X86_64
#       define EM_X86_64 62
#   endif
#   define ICU_ENTRY_OFFSET 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include "unicode/putil.h"
#include "cmemory.h"
#include "cstring.h"
#include "filestrm.h"
#include "toolutil.h"
#include "unicode/uclean.h"
#include "uoptions.h"

#define MAX_COLUMN ((uint32_t)(0xFFFFFFFFU))

static uint32_t column=MAX_COLUMN;

#if defined(U_WINDOWS) || defined(U_ELF)
#define CAN_GENERATE_OBJECTS
#endif

/* prototypes --------------------------------------------------------------- */

static void
writeCCode(const char *filename, const char *destdir);

static void
writeAssemblyCode(const char *filename, const char *destdir);

#ifdef CAN_GENERATE_OBJECTS
static void
writeObjectCode(const char *filename, const char *destdir);
#endif

static void
getOutFilename(const char *inFilename, const char *destdir, char *outFilename, char *entryName, const char *newSuffix);

static void
write8(FileStream *out, uint8_t byte);

static void
write32(FileStream *out, uint32_t byte);

#ifdef OS400
static void
write8str(FileStream *out, uint8_t byte);
#endif
/* -------------------------------------------------------------------------- */

enum { 
  kOptHelpH = 0,
  kOptHelpQuestionMark,
  kOptDestDir,
  kOptName,
  kOptEntryPoint,
#ifdef CAN_GENERATE_OBJECTS
  kOptObject,
  kOptMatchArch,
#endif
  kOptFilename,
  kOptAssembly
};

/*
Creating Template Files for New Platforms

Let the cc compiler help you get started.
Compile this program
    const unsigned int x[5] = {1, 2, 0xdeadbeef, 0xffffffff, 16};
with the -S option to produce assembly output.

For example, this will generate array.s:
gcc -S array.c

This will produce a .s file that may look like this:

    .file   "array.c"
    .version        "01.01"
gcc2_compiled.:
    .globl x
    .section        .rodata
    .align 4
    .type    x,@object
    .size    x,20
x:
    .long   1
    .long   2
    .long   -559038737
    .long   -1
    .long   16
    .ident  "GCC: (GNU) 2.96 20000731 (Red Hat Linux 7.1 2.96-85)"

which gives a starting point that will compile, and can be transformed
to become the template, generally with some consulting of as docs and
some experimentation.

If you want ICU to automatically use this assembly, you should
specify "GENCCODE_ASSEMBLY=-a name" in the specific config/mh-* file,
where the name is the compiler or platform that you used in this
assemblyHeader data structure.
*/
static const struct AssemblyType {
    const char *name;
    const char *header;
    const char *beginLine;
} assemblyHeader[] = {
    {"gcc",
        ".globl %s\n"
        "\t.section .note.GNU-stack,\"\",@progbits\n"
        "\t.section .rodata\n"
        "\t.align 8\n" /* Either align 8 bytes or 2^8 (256) bytes. 8 bytes is needed. */
        "\t.type %s,@object\n"
        "%s:\n\n",

        ".long "
    },
    {"gcc-darwin",
        /*"\t.section __TEXT,__text,regular,pure_instructions\n"
        "\t.section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32\n"*/
        ".globl _%s\n"
        "\t.data\n"
        "\t.const\n"
        "\t.align 4\n"  /* 1<<4 = 16 */
        "_%s:\n\n",

        ".long "
    },
    {"gcc-cygwin",
        ".globl _%s\n"
        "\t.section .rodata\n"
        "\t.align 8\n" /* Either align 8 bytes or 2^8 (256) bytes. 8 bytes is needed. */
        "_%s:\n\n",

        ".long "
    },
    {"sun",
        "\t.section \".rodata\"\n"
        "\t.align   8\n"
        ".globl     %s\n"
        "%s:\n",

        ".word "
    },
    {"sun86",
        "Drodata.rodata:\n"
        "\t.type   Drodata.rodata,@object\n"
        "\t.size   Drodata.rodata,0\n"
        "\t.globl  %s\n"
        "\t.align  8\n"
        "%s:\n",

        ".4byte "
    },
    {"xlc",
        ".globl %s{RO}\n"
        "\t.toc\n"
        "%s:\n"
        "\t.csect %s{RO}, 4\n",

        ".long "
    },
    {"aCC",
        "\t.SPACE  $TEXT$\n"
        "\t.SUBSPA $LIT$\n"
        "%s\n"
        "\t.EXPORT %s\n"
        "\t.ALIGN  16\n",

        ".WORD "
    }
};

static int32_t assemblyHeaderIndex = -1;

static UOption options[]={
/*0*/UOPTION_HELP_H,
     UOPTION_HELP_QUESTION_MARK,
     UOPTION_DESTDIR,
     UOPTION_DEF("name", 'n', UOPT_REQUIRES_ARG),
     UOPTION_DEF("entrypoint", 'e', UOPT_REQUIRES_ARG),
#ifdef CAN_GENERATE_OBJECTS
/*5*/UOPTION_DEF("object", 'o', UOPT_NO_ARG),
     UOPTION_DEF("match-arch", 'm', UOPT_REQUIRES_ARG),
#endif
     UOPTION_DEF("filename", 'f', UOPT_REQUIRES_ARG),
     UOPTION_DEF("assembly", 'a', UOPT_REQUIRES_ARG)
};

extern int
main(int argc, char* argv[]) {
    UBool verbose = TRUE;
    int32_t idx;

    U_MAIN_INIT_ARGS(argc, argv);

    options[kOptDestDir].value = ".";

    /* read command line options */
    argc=u_parseArgs(argc, argv, sizeof(options)/sizeof(options[0]), options);

    /* error handling, printing usage message */
    if(argc<0) {
        fprintf(stderr,
            "error in command line argument \"%s\"\n",
            argv[-argc]);
    }
    if(argc<0 || options[kOptHelpH].doesOccur || options[kOptHelpQuestionMark].doesOccur) {
        fprintf(stderr,
            "usage: %s [-options] filename1 filename2 ...\n"
            "\tread each binary input file and \n"
            "\tcreate a .c file with a byte array that contains the input file's data\n"
            "options:\n"
            "\t-h or -? or --help  this usage text\n"
            "\t-d or --destdir     destination directory, followed by the path\n"
            "\t-n or --name        symbol prefix, followed by the prefix\n"
            "\t-e or --entrypoint  entry point name, followed by the name (_dat will be appended)\n"
            "\t-r or --revision    Specify a version\n"
            , argv[0]);
#ifdef CAN_GENERATE_OBJECTS
        fprintf(stderr,
            "\t-o or --object      write a .obj file instead of .c\n"
            "\t-m or --match-arch file.o  match the architecture (CPU, 32/64 bits) of the specified .o\n"
            "\t                    ELF format defaults to i386. Windows defaults to the native platform.\n");
#endif
        fprintf(stderr,
            "\t-f or --filename    Specify an alternate base filename. (default: symbolname_typ)\n"
            "\t-a or --assembly    Create assembly file. (possible values are: ");

        fprintf(stderr, "%s", assemblyHeader[0].name);
        for (idx = 1; idx < (int32_t)(sizeof(assemblyHeader)/sizeof(assemblyHeader[0])); idx++) {
            fprintf(stderr, ", %s", assemblyHeader[idx].name);
        }
        fprintf(stderr,
            ")\n");
    } else {
        const char *message, *filename;
        void (*writeCode)(const char *, const char *);

        if(options[kOptAssembly].doesOccur) {
            message="generating assembly code for %s\n";
            writeCode=&writeAssemblyCode;
            for (idx = 0; idx < (int32_t)(sizeof(assemblyHeader)/sizeof(assemblyHeader[0])); idx++) {
                if (uprv_strcmp(options[kOptAssembly].value, assemblyHeader[idx].name) == 0) {
                    assemblyHeaderIndex = idx;
                    break;
                }
            }
            if (assemblyHeaderIndex < 0) {
                fprintf(stderr,
                    "Assembly type \"%s\" is unknown.\n", options[kOptAssembly].value);
                return -1;
            }
        }
#ifdef CAN_GENERATE_OBJECTS
        else if(options[kOptObject].doesOccur) {
            message="generating object code for %s\n";
            writeCode=&writeObjectCode;
        }
#endif
        else
        {
            message="generating C code for %s\n";
            writeCode=&writeCCode;
        }
        while(--argc) {
            filename=getLongPathname(argv[argc]);
            if (verbose) {
                fprintf(stdout, message, filename);
            }
            column=MAX_COLUMN;
            writeCode(filename, options[kOptDestDir].value);
        }
    }

    return 0;
}

static void
writeAssemblyCode(const char *filename, const char *destdir) {
    char entry[64];
    uint32_t buffer[1024];
    char *bufferStr = (char *)buffer;
    FileStream *in, *out;
    size_t i, length;

    in=T_FileStream_open(filename, "rb");
    if(in==NULL) {
        fprintf(stderr, "genccode: unable to open input file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    getOutFilename(filename, destdir, bufferStr, entry, ".s");
    out=T_FileStream_open(bufferStr, "w");
    if(out==NULL) {
        fprintf(stderr, "genccode: unable to open output file %s\n", bufferStr);
        exit(U_FILE_ACCESS_ERROR);
    }

    if(options[kOptEntryPoint].doesOccur) {
        uprv_strcpy(entry, options[kOptEntryPoint].value);
        uprv_strcat(entry, "_dat");
    }

    /* turn dashes or dots in the entry name into underscores */
    length=uprv_strlen(entry);
    for(i=0; i<length; ++i) {
        if(entry[i]=='-' || entry[i]=='.') {
            entry[i]='_';
        }
    }

    sprintf(bufferStr, assemblyHeader[assemblyHeaderIndex].header,
        entry, entry, entry, entry,
        entry, entry, entry, entry);
    T_FileStream_writeLine(out, bufferStr);
    T_FileStream_writeLine(out, assemblyHeader[assemblyHeaderIndex].beginLine);

    for(;;) {
        length=T_FileStream_read(in, buffer, sizeof(buffer));
        if(length==0) {
            break;
        }
        if (length != sizeof(buffer)) {
            /* pad with extra 0's when at the end of the file */
            for(i=0; i < (length % sizeof(uint32_t)); ++i) {
                buffer[length+i] = 0;
            }
        }
        for(i=0; i<(length/sizeof(buffer[0])); i++) {
            write32(out, buffer[i]);
        }
    }

    T_FileStream_writeLine(out, "\n");

    if(T_FileStream_error(in)) {
        fprintf(stderr, "genccode: file read error while generating from file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    if(T_FileStream_error(out)) {
        fprintf(stderr, "genccode: file write error while generating from file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    T_FileStream_close(out);
    T_FileStream_close(in);
}

static void
writeCCode(const char *filename, const char *destdir) {
    char buffer[4096], entry[64];
    FileStream *in, *out;
    size_t i, length;

    in=T_FileStream_open(filename, "rb");
    if(in==NULL) {
        fprintf(stderr, "genccode: unable to open input file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    if(options[kOptName].doesOccur) { /* prepend  'icudt28_' */
      strcpy(entry, options[kOptName].value);
      strcat(entry, "_");
    } else {
      entry[0] = 0;
    }

    getOutFilename(filename, destdir, buffer, entry+uprv_strlen(entry), ".c");
    out=T_FileStream_open(buffer, "w");
    if(out==NULL) {
        fprintf(stderr, "genccode: unable to open output file %s\n", buffer);
        exit(U_FILE_ACCESS_ERROR);
    }

    /* turn dashes or dots in the entry name into underscores */
    length=uprv_strlen(entry);
    for(i=0; i<length; ++i) {
        if(entry[i]=='-' || entry[i]=='.') {
            entry[i]='_';
        }
    }
    
#ifdef OS400
    /*
    TODO: Fix this once the compiler implements this feature. Keep in sync with udatamem.c

    This is here because this platform can't currently put
    const data into the read-only pages of an object or
    shared library (service program). Only strings are allowed in read-only
    pages, so we use char * strings to store the data.

    In order to prevent the beginning of the data from ever matching the
    magic numbers we must still use the initial double.
    [grhoten 4/24/2003]
    */
    sprintf(buffer,
        "#define U_DISABLE_RENAMING 1\n"
        "#include \"unicode/umachine.h\"\n"
        "U_CDECL_BEGIN\n"
        "const struct {\n"
        "    double bogus;\n"
        "    const char *bytes; \n"
        "} %s={ 0.0, \n",
        entry);
    T_FileStream_writeLine(out, buffer);

    for(;;) {
        length=T_FileStream_read(in, buffer, sizeof(buffer));
        if(length==0) {
            break;
        }
        for(i=0; i<length; ++i) {
            write8str(out, (uint8_t)buffer[i]);
        }
    }

    T_FileStream_writeLine(out, "\"\n};\nU_CDECL_END\n");
#else
    /* Function renaming shouldn't be done in data */
    sprintf(buffer,
        "#define U_DISABLE_RENAMING 1\n"
        "#include \"unicode/umachine.h\"\n"
        "U_CDECL_BEGIN\n"
        "const struct {\n"
        "    double bogus;\n"
        "    uint8_t bytes[%ld]; \n"
        "} %s={ 0.0, {\n",
        (long)T_FileStream_size(in), entry);
    T_FileStream_writeLine(out, buffer);

    for(;;) {
        length=T_FileStream_read(in, buffer, sizeof(buffer));
        if(length==0) {
            break;
        }
        for(i=0; i<length; ++i) {
            write8(out, (uint8_t)buffer[i]);
        }
    }

    T_FileStream_writeLine(out, "\n}\n};\nU_CDECL_END\n");
#endif

    if(T_FileStream_error(in)) {
        fprintf(stderr, "genccode: file read error while generating from file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    if(T_FileStream_error(out)) {
        fprintf(stderr, "genccode: file write error while generating from file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    T_FileStream_close(out);
    T_FileStream_close(in);
}

#ifdef CAN_GENERATE_OBJECTS
static void
getArchitecture(uint16_t *pCPU, uint16_t *pBits, UBool *pIsBigEndian) {
    int64_t buffer[256];
    const char *filename;
    FileStream *in;
    int32_t length;

#ifdef U_ELF
    /* Pointer to ELF header. Elf32_Ehdr and ELF64_Ehdr are identical for the necessary fields. */
    const Elf32_Ehdr *pHeader32;
#elif defined(U_WINDOWS)
    const IMAGE_FILE_HEADER *pHeader;
#else
#   error "Unknown platform for CAN_GENERATE_OBJECTS."
#endif

    if(options[kOptMatchArch].doesOccur) {
        filename=options[kOptMatchArch].value;
    } else {
        /* set defaults */
#ifdef U_ELF
        /* set EM_386 because elf.h does not provide better defaults */
        *pCPU=EM_386;
        *pBits=32;
        *pIsBigEndian=(UBool)(U_IS_BIG_ENDIAN ? ELFDATA2MSB : ELFDATA2LSB);
#elif defined(U_WINDOWS)
/* _M_IA64 should be defined in windows.h */
#   if defined(_M_IA64)
        *pCPU=IMAGE_FILE_MACHINE_IA64;
#   elif defined(_M_AMD64)
        *pCPU=IMAGE_FILE_MACHINE_AMD64;
#   else
        *pCPU=IMAGE_FILE_MACHINE_I386;
#   endif
        *pBits= *pCPU==IMAGE_FILE_MACHINE_I386 ? 32 : 64;
        *pIsBigEndian=FALSE;
#else
#   error "Unknown platform for CAN_GENERATE_OBJECTS."
#endif
        return;
    }

    in=T_FileStream_open(filename, "rb");
    if(in==NULL) {
        fprintf(stderr, "genccode: unable to open match-arch file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }
    length=T_FileStream_read(in, buffer, sizeof(buffer));

#ifdef U_ELF
    if(length<sizeof(Elf32_Ehdr)) {
        fprintf(stderr, "genccode: match-arch file %s is too short\n", filename);
        exit(U_UNSUPPORTED_ERROR);
    }
    pHeader32=(const Elf32_Ehdr *)buffer;
    if(
        pHeader32->e_ident[0]!=ELFMAG0 ||
        pHeader32->e_ident[1]!=ELFMAG1 ||
        pHeader32->e_ident[2]!=ELFMAG2 ||
        pHeader32->e_ident[3]!=ELFMAG3 ||
        pHeader32->e_ident[EI_CLASS]<ELFCLASS32 || pHeader32->e_ident[EI_CLASS]>ELFCLASS64
    ) {
        fprintf(stderr, "genccode: match-arch file %s is not an ELF object file, or not supported\n", filename);
        exit(U_UNSUPPORTED_ERROR);
    }

    *pBits= pHeader32->e_ident[EI_CLASS]==ELFCLASS32 ? 32 : 64; /* only 32 or 64: see check above */
#ifdef U_ELF64
    if(*pBits!=32 && *pBits!=64) {
        fprintf(stderr, "genccode: currently only supports 32-bit and 64-bit ELF format\n");
        exit(U_UNSUPPORTED_ERROR);
    }
#else
    if(*pBits!=32) {
        fprintf(stderr, "genccode: built with elf.h missing 64-bit definitions\n");
        exit(U_UNSUPPORTED_ERROR);
    }
#endif

    *pIsBigEndian=(UBool)(pHeader32->e_ident[EI_DATA]==ELFDATA2MSB);
    if(*pIsBigEndian!=U_IS_BIG_ENDIAN) {
        fprintf(stderr, "genccode: currently only same-endianness ELF formats are supported\n");
        exit(U_UNSUPPORTED_ERROR);
    }
    /* TODO: Support byte swapping */

    *pCPU=pHeader32->e_machine;
#elif defined(U_WINDOWS)
    if(length<sizeof(IMAGE_FILE_HEADER)) {
        fprintf(stderr, "genccode: match-arch file %s is too short\n", filename);
        exit(U_UNSUPPORTED_ERROR);
    }
    pHeader=(const IMAGE_FILE_HEADER *)buffer;
    *pCPU=pHeader->Machine;
    /*
     * The number of bits is implicit with the Machine value.
     * *pBits is ignored in the calling code, so this need not be precise.
     */
    *pBits= *pCPU==IMAGE_FILE_MACHINE_I386 ? 32 : 64;
    /* Windows always runs on little-endian CPUs. */
    *pIsBigEndian=FALSE;
#else
#   error "Unknown platform for CAN_GENERATE_OBJECTS."
#endif

    T_FileStream_close(in);
}

static void
writeObjectCode(const char *filename, const char *destdir) {
    /* common variables */
    char buffer[4096], entry[40]={ 0 };
    FileStream *in, *out;
    const char *newSuffix;
    int32_t i, entryLength, length, size, entryOffset=0, entryLengthOffset=0;

    uint16_t cpu, bits;
    UBool makeBigEndian;

    /* platform-specific variables and initialization code */
#ifdef U_ELF
    /* 32-bit Elf file header */
    static Elf32_Ehdr header32={
        {
            /* e_ident[] */
            ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3,
            ELFCLASS32,
            U_IS_BIG_ENDIAN ? ELFDATA2MSB : ELFDATA2LSB,
            EV_CURRENT /* EI_VERSION */
        },
        ET_REL,
        EM_386,
        EV_CURRENT, /* e_version */
        0, /* e_entry */
        0, /* e_phoff */
        (Elf32_Off)sizeof(Elf32_Ehdr), /* e_shoff */
        0, /* e_flags */
        (Elf32_Half)sizeof(Elf32_Ehdr), /* eh_size */
        0, /* e_phentsize */
        0, /* e_phnum */
        (Elf32_Half)sizeof(Elf32_Shdr), /* e_shentsize */
        5, /* e_shnum */
        2 /* e_shstrndx */
    };

    /* 32-bit Elf section header table */
    static Elf32_Shdr sectionHeaders32[5]={
        { /* SHN_UNDEF */
            0
        },
        { /* .symtab */
            1, /* sh_name */
            SHT_SYMTAB,
            0, /* sh_flags */
            0, /* sh_addr */
            (Elf32_Off)(sizeof(header32)+sizeof(sectionHeaders32)), /* sh_offset */
            (Elf32_Word)(2*sizeof(Elf32_Sym)), /* sh_size */
            3, /* sh_link=sect hdr index of .strtab */
            1, /* sh_info=One greater than the symbol table index of the last
                * local symbol (with STB_LOCAL). */
            4, /* sh_addralign */
            (Elf32_Word)(sizeof(Elf32_Sym)) /* sh_entsize */
        },
        { /* .shstrtab */
            9, /* sh_name */
            SHT_STRTAB,
            0, /* sh_flags */
            0, /* sh_addr */
            (Elf32_Off)(sizeof(header32)+sizeof(sectionHeaders32)+2*sizeof(Elf32_Sym)), /* sh_offset */
            40, /* sh_size */
            0, /* sh_link */
            0, /* sh_info */
            1, /* sh_addralign */
            0 /* sh_entsize */
        },
        { /* .strtab */
            19, /* sh_name */
            SHT_STRTAB,
            0, /* sh_flags */
            0, /* sh_addr */
            (Elf32_Off)(sizeof(header32)+sizeof(sectionHeaders32)+2*sizeof(Elf32_Sym)+40), /* sh_offset */
            (Elf32_Word)sizeof(entry), /* sh_size */
            0, /* sh_link */
            0, /* sh_info */
            1, /* sh_addralign */
            0 /* sh_entsize */
        },
        { /* .rodata */
            27, /* sh_name */
            SHT_PROGBITS,
            SHF_ALLOC, /* sh_flags */
            0, /* sh_addr */
            (Elf32_Off)(sizeof(header32)+sizeof(sectionHeaders32)+2*sizeof(Elf32_Sym)+40+sizeof(entry)), /* sh_offset */
            0, /* sh_size */
            0, /* sh_link */
            0, /* sh_info */
            16, /* sh_addralign */
            0 /* sh_entsize */
        }
    };

    /* symbol table */
    static Elf32_Sym symbols32[2]={
        { /* STN_UNDEF */
            0
        },
        { /* data entry point */
            1, /* st_name */
            0, /* st_value */
            0, /* st_size */
            ELF64_ST_INFO(STB_GLOBAL, STT_OBJECT),
            0, /* st_other */
            4 /* st_shndx=index of related section table entry */
        }
    };

    /* section header string table, with decimal string offsets */
    static const char sectionStrings[40]=
        /*  0 */ "\0"
        /*  1 */ ".symtab\0"
        /*  9 */ ".shstrtab\0"
        /* 19 */ ".strtab\0"
        /* 27 */ ".rodata\0"
        /* 35 */ "\0\0\0\0"; /* contains terminating NUL */
        /* 40: padded to multiple of 8 bytes */

    /*
     * Use entry[] for the string table which will contain only the
     * entry point name.
     * entry[0] must be 0 (NUL)
     * The entry point name can be up to 38 characters long (sizeof(entry)-2).
     */

    /* 16-align .rodata in the .o file, just in case */
    static const char padding[16]={ 0 };
    int32_t paddingSize;

#ifdef U_ELF64
    /* 64-bit Elf file header */
    static Elf64_Ehdr header64={
        {
            /* e_ident[] */
            ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3,
            ELFCLASS64,
            U_IS_BIG_ENDIAN ? ELFDATA2MSB : ELFDATA2LSB,
            EV_CURRENT /* EI_VERSION */
        },
        ET_REL,
        EM_X86_64,
        EV_CURRENT, /* e_version */
        0, /* e_entry */
        0, /* e_phoff */
        (Elf64_Off)sizeof(Elf64_Ehdr), /* e_shoff */
        0, /* e_flags */
        (Elf64_Half)sizeof(Elf64_Ehdr), /* eh_size */
        0, /* e_phentsize */
        0, /* e_phnum */
        (Elf64_Half)sizeof(Elf64_Shdr), /* e_shentsize */
        5, /* e_shnum */
        2 /* e_shstrndx */
    };

    /* 64-bit Elf section header table */
    static Elf64_Shdr sectionHeaders64[5]={
        { /* SHN_UNDEF */
            0
        },
        { /* .symtab */
            1, /* sh_name */
            SHT_SYMTAB,
            0, /* sh_flags */
            0, /* sh_addr */
            (Elf64_Off)(sizeof(header64)+sizeof(sectionHeaders64)), /* sh_offset */
            (Elf64_Xword)(2*sizeof(Elf64_Sym)), /* sh_size */
            3, /* sh_link=sect hdr index of .strtab */
            1, /* sh_info=One greater than the symbol table index of the last
                * local symbol (with STB_LOCAL). */
            4, /* sh_addralign */
            (Elf64_Xword)(sizeof(Elf64_Sym)) /* sh_entsize */
        },
        { /* .shstrtab */
            9, /* sh_name */
            SHT_STRTAB,
            0, /* sh_flags */
            0, /* sh_addr */
            (Elf64_Off)(sizeof(header64)+sizeof(sectionHeaders64)+2*sizeof(Elf64_Sym)), /* sh_offset */
            40, /* sh_size */
            0, /* sh_link */
            0, /* sh_info */
            1, /* sh_addralign */
            0 /* sh_entsize */
        },
        { /* .strtab */
            19, /* sh_name */
            SHT_STRTAB,
            0, /* sh_flags */
            0, /* sh_addr */
            (Elf64_Off)(sizeof(header64)+sizeof(sectionHeaders64)+2*sizeof(Elf64_Sym)+40), /* sh_offset */
            (Elf64_Xword)sizeof(entry), /* sh_size */
            0, /* sh_link */
            0, /* sh_info */
            1, /* sh_addralign */
            0 /* sh_entsize */
        },
        { /* .rodata */
            27, /* sh_name */
            SHT_PROGBITS,
            SHF_ALLOC, /* sh_flags */
            0, /* sh_addr */
            (Elf64_Off)(sizeof(header64)+sizeof(sectionHeaders64)+2*sizeof(Elf64_Sym)+40+sizeof(entry)), /* sh_offset */
            0, /* sh_size */
            0, /* sh_link */
            0, /* sh_info */
            16, /* sh_addralign */
            0 /* sh_entsize */
        }
    };

    /*
     * 64-bit symbol table
     * careful: different order of items compared with Elf32_sym!
     */
    static Elf64_Sym symbols64[2]={
        { /* STN_UNDEF */
            0
        },
        { /* data entry point */
            1, /* st_name */
            ELF64_ST_INFO(STB_GLOBAL, STT_OBJECT),
            0, /* st_other */
            4, /* st_shndx=index of related section table entry */
            0, /* st_value */
            0 /* st_size */
        }
    };

#endif /* U_ELF64 */

    /* entry[] have a leading NUL */
    entryOffset=1;

    /* in the common code, count entryLength from after the NUL */
    entryLengthOffset=1;

    newSuffix=".o";

#elif defined(U_WINDOWS)
    struct {
        IMAGE_FILE_HEADER fileHeader;
        IMAGE_SECTION_HEADER sections[2];
        char linkerOptions[100];
    } objHeader;
    IMAGE_SYMBOL symbols[1];
    struct {
        DWORD sizeofLongNames;
        char longNames[100];
    } symbolNames;

    /*
     * entry sometimes have a leading '_'
     * overwritten if entryOffset==0 depending on the target platform
     * see check for cpu below
     */
    entry[0]='_';

    newSuffix=".obj";
#else
#   error "Unknown platform for CAN_GENERATE_OBJECTS."
#endif

    /* deal with options, files and the entry point name */
    getArchitecture(&cpu, &bits, &makeBigEndian);
    printf("genccode: --match-arch cpu=%hu bits=%hu big-endian=%hu\n", cpu, bits, makeBigEndian);
#ifdef U_WINDOWS
    if(cpu==IMAGE_FILE_MACHINE_I386) {
        entryOffset=1;
    }
#endif

    in=T_FileStream_open(filename, "rb");
    if(in==NULL) {
        fprintf(stderr, "genccode: unable to open input file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }
    size=T_FileStream_size(in);

    getOutFilename(filename, destdir, buffer, entry+entryOffset, newSuffix);

    if(options[kOptEntryPoint].doesOccur) {
        uprv_strcpy(entry+entryOffset, options[kOptEntryPoint].value);
        uprv_strcat(entry+entryOffset, "_dat");
    }
    /* turn dashes in the entry name into underscores */
    entryLength=(int32_t)uprv_strlen(entry+entryLengthOffset);
    for(i=0; i<entryLength; ++i) {
        if(entry[entryLengthOffset+i]=='-') {
            entry[entryLengthOffset+i]='_';
        }
    }

    /* open the output file */
    out=T_FileStream_open(buffer, "wb");
    if(out==NULL) {
        fprintf(stderr, "genccode: unable to open output file %s\n", buffer);
        exit(U_FILE_ACCESS_ERROR);
    }

#ifdef U_ELF
    if(bits==32) {
        header32.e_ident[EI_DATA]= makeBigEndian ? ELFDATA2MSB : ELFDATA2LSB;
        header32.e_machine=cpu;

        /* 16-align .rodata in the .o file, just in case */
        paddingSize=sectionHeaders32[4].sh_offset & 0xf;
        if(paddingSize!=0) {
                paddingSize=0x10-paddingSize;
                sectionHeaders32[4].sh_offset+=paddingSize;
        }

        sectionHeaders32[4].sh_size=(Elf32_Word)size;

        symbols32[1].st_size=(Elf32_Word)size;

        /* write .o headers */
        T_FileStream_write(out, &header32, (int32_t)sizeof(header32));
        T_FileStream_write(out, sectionHeaders32, (int32_t)sizeof(sectionHeaders32));
        T_FileStream_write(out, symbols32, (int32_t)sizeof(symbols32));
    } else /* bits==64 */ {
#ifdef U_ELF64
        header64.e_ident[EI_DATA]= makeBigEndian ? ELFDATA2MSB : ELFDATA2LSB;
        header64.e_machine=cpu;

        /* 16-align .rodata in the .o file, just in case */
        paddingSize=sectionHeaders64[4].sh_offset & 0xf;
        if(paddingSize!=0) {
                paddingSize=0x10-paddingSize;
                sectionHeaders64[4].sh_offset+=paddingSize;
        }

        sectionHeaders64[4].sh_size=(Elf64_Xword)size;

        symbols64[1].st_size=(Elf64_Xword)size;

        /* write .o headers */
        T_FileStream_write(out, &header64, (int32_t)sizeof(header64));
        T_FileStream_write(out, sectionHeaders64, (int32_t)sizeof(sectionHeaders64));
        T_FileStream_write(out, symbols64, (int32_t)sizeof(symbols64));
#endif
    }

    T_FileStream_write(out, sectionStrings, (int32_t)sizeof(sectionStrings));
    T_FileStream_write(out, entry, (int32_t)sizeof(entry));
    if(paddingSize!=0) {
        T_FileStream_write(out, padding, paddingSize);
    }
#elif defined(U_WINDOWS)
    /* populate the .obj headers */
    uprv_memset(&objHeader, 0, sizeof(objHeader));
    uprv_memset(&symbols, 0, sizeof(symbols));
    uprv_memset(&symbolNames, 0, sizeof(symbolNames));

    /* write the linker export directive */
    uprv_strcpy(objHeader.linkerOptions, "-export:");
    length=8;
    uprv_strcpy(objHeader.linkerOptions+length, entry);
    length+=entryLength;
    uprv_strcpy(objHeader.linkerOptions+length, ",data ");
    length+=6;

    /* set the file header */
    objHeader.fileHeader.Machine=cpu;
    objHeader.fileHeader.NumberOfSections=2;
    objHeader.fileHeader.TimeDateStamp=(DWORD)time(NULL);
    objHeader.fileHeader.PointerToSymbolTable=IMAGE_SIZEOF_FILE_HEADER+2*IMAGE_SIZEOF_SECTION_HEADER+length+size; /* start of symbol table */
    objHeader.fileHeader.NumberOfSymbols=1;

    /* set the section for the linker options */
    uprv_strncpy((char *)objHeader.sections[0].Name, ".drectve", 8);
    objHeader.sections[0].SizeOfRawData=length;
    objHeader.sections[0].PointerToRawData=IMAGE_SIZEOF_FILE_HEADER+2*IMAGE_SIZEOF_SECTION_HEADER;
    objHeader.sections[0].Characteristics=IMAGE_SCN_LNK_INFO|IMAGE_SCN_LNK_REMOVE|IMAGE_SCN_ALIGN_1BYTES;

    /* set the data section */
    uprv_strncpy((char *)objHeader.sections[1].Name, ".rdata", 6);
    objHeader.sections[1].SizeOfRawData=size;
    objHeader.sections[1].PointerToRawData=IMAGE_SIZEOF_FILE_HEADER+2*IMAGE_SIZEOF_SECTION_HEADER+length;
    objHeader.sections[1].Characteristics=IMAGE_SCN_CNT_INITIALIZED_DATA|IMAGE_SCN_ALIGN_16BYTES|IMAGE_SCN_MEM_READ;

    /* set the symbol table */
    if(entryLength<=8) {
        uprv_strncpy((char *)symbols[0].N.ShortName, entry, entryLength);
        symbolNames.sizeofLongNames=4;
    } else {
        symbols[0].N.Name.Short=0;
        symbols[0].N.Name.Long=4;
        symbolNames.sizeofLongNames=4+entryLength+1;
        uprv_strcpy(symbolNames.longNames, entry);
    }
    symbols[0].SectionNumber=2;
    symbols[0].StorageClass=IMAGE_SYM_CLASS_EXTERNAL;

    /* write the file header and the linker options section */
    T_FileStream_write(out, &objHeader, objHeader.sections[1].PointerToRawData);
#else
#   error "Unknown platform for CAN_GENERATE_OBJECTS."
#endif

    /* copy the data file into section 2 */
    for(;;) {
        length=T_FileStream_read(in, buffer, sizeof(buffer));
        if(length==0) {
            break;
        }
        T_FileStream_write(out, buffer, (int32_t)length);
    }

#ifdef U_WINDOWS
    /* write the symbol table */
    T_FileStream_write(out, symbols, IMAGE_SIZEOF_SYMBOL);
    T_FileStream_write(out, &symbolNames, symbolNames.sizeofLongNames);
#endif

    if(T_FileStream_error(in)) {
        fprintf(stderr, "genccode: file read error while generating from file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    if(T_FileStream_error(out)) {
        fprintf(stderr, "genccode: file write error while generating from file %s\n", filename);
        exit(U_FILE_ACCESS_ERROR);
    }

    T_FileStream_close(out);
    T_FileStream_close(in);
}
#endif

static void
getOutFilename(const char *inFilename, const char *destdir, char *outFilename, char *entryName, const char *newSuffix) {
    const char *basename=findBasename(inFilename), *suffix=uprv_strrchr(basename, '.');

    /* copy path */
    if(destdir!=NULL && *destdir!=0) {
        do {
            *outFilename++=*destdir++;
        } while(*destdir!=0);
        if(*(outFilename-1)!=U_FILE_SEP_CHAR) {
            *outFilename++=U_FILE_SEP_CHAR;
        }
        inFilename=basename;
    } else {
        while(inFilename<basename) {
            *outFilename++=*inFilename++;
        }
    }

    if(suffix==NULL) {
        /* the filename does not have a suffix */
        uprv_strcpy(entryName, inFilename);
        if(options[kOptFilename].doesOccur) {
          uprv_strcpy(outFilename, options[kOptFilename].value);
        } else {
          uprv_strcpy(outFilename, inFilename);
        }
        uprv_strcat(outFilename, newSuffix);
    } else {
        char *saveOutFilename = outFilename;
        /* copy basename */
        while(inFilename<suffix) {
            if(*inFilename=='-') {
                /* iSeries cannot have '-' in the .o objects. */
                *outFilename++=*entryName++='_';
                inFilename++;
            }
            else {
                *outFilename++=*entryName++=*inFilename++;
            }
        }

        /* replace '.' by '_' */
        *outFilename++=*entryName++='_';
        ++inFilename;

        /* copy suffix */
        while(*inFilename!=0) {
            *outFilename++=*entryName++=*inFilename++;
        }

        *entryName=0;

        if(options[kOptFilename].doesOccur) {
            uprv_strcpy(saveOutFilename, options[kOptFilename].value);
            uprv_strcat(saveOutFilename, newSuffix); 
        } else {
            /* add ".c" */
            uprv_strcpy(outFilename, newSuffix);
        }
    }
}

static void
write32(FileStream *out, uint32_t bitField) {
    int32_t i;
    char bitFieldStr[64]; /* This is more bits than needed for a 32-bit number */
    char *s = bitFieldStr;
    uint8_t *ptrIdx = (uint8_t *)&bitField;
    static const char hexToStr[16] = {
        '0','1','2','3',
        '4','5','6','7',
        '8','9','A','B',
        'C','D','E','F'
    };

    /* write the value, possibly with comma and newline */
    if(column==MAX_COLUMN) {
        /* first byte */
        column=1;
    } else if(column<32) {
        *(s++)=',';
        ++column;
    } else {
        *(s++)='\n';
        uprv_strcpy(s, assemblyHeader[assemblyHeaderIndex].beginLine);
        s+=uprv_strlen(s);
        column=1;
    }

    if (bitField < 10) {
        /* It's a small number. Don't waste the space for 0x */
        *(s++)=hexToStr[bitField];
    }
    else {
        int seenNonZero = 0; /* This is used to remove leading zeros */

        *(s++)='0';
        *(s++)='x';

        /* This creates a 32-bit field */
#if U_IS_BIG_ENDIAN
        for (i = 0; i < sizeof(uint32_t); i++)
#else
        for (i = sizeof(uint32_t)-1; i >= 0 ; i--)
#endif
        {
            uint8_t value = ptrIdx[i];
            if (value || seenNonZero) {
                *(s++)=hexToStr[value>>4];
                *(s++)=hexToStr[value&0xF];
                seenNonZero = 1;
            }
        }
    }

    *(s++)=0;
    T_FileStream_writeLine(out, bitFieldStr);
}

static void
write8(FileStream *out, uint8_t byte) {
    char s[4];
    int i=0;

    /* convert the byte value to a string */
    if(byte>=100) {
        s[i++]=(char)('0'+byte/100);
        byte%=100;
    }
    if(i>0 || byte>=10) {
        s[i++]=(char)('0'+byte/10);
        byte%=10;
    }
    s[i++]=(char)('0'+byte);
    s[i]=0;

    /* write the value, possibly with comma and newline */
    if(column==MAX_COLUMN) {
        /* first byte */
        column=1;
    } else if(column<16) {
        T_FileStream_writeLine(out, ",");
        ++column;
    } else {
        T_FileStream_writeLine(out, ",\n");
        column=1;
    }
    T_FileStream_writeLine(out, s);
}

#ifdef OS400
static void
write8str(FileStream *out, uint8_t byte) {
    char s[8];

    if (byte > 7)
        sprintf(s, "\\x%X", byte);
    else
        sprintf(s, "\\%X", byte);

    /* write the value, possibly with comma and newline */
    if(column==MAX_COLUMN) {
        /* first byte */
        column=1;
        T_FileStream_writeLine(out, "\"");
    } else if(column<24) {
        ++column;
    } else {
        T_FileStream_writeLine(out, "\"\n\"");
        column=1;
    }
    T_FileStream_writeLine(out, s);
}
#endif