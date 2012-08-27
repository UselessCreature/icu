/*
**********************************************************************
*   Copyright (C) 1997-2012, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*
* File UMUTEX.H
*
* Modification History:
*
*   Date        Name        Description
*   04/02/97  aliu        Creation.
*   04/07/99  srl         rewrite - C interface, multiple mutices
*   05/13/99  stephen     Changed to umutex (from cmutex)
******************************************************************************
*/

#ifndef UMUTEX_H
#define UMUTEX_H

#include "unicode/utypes.h"
#include "unicode/uclean.h"
#include "putilimp.h"

// TODO: Test only, remove this.
#define U_HAVE_GCC_ATOMICS 1

#if defined(_MSC_VER) && _MSC_VER >= 1500
# include <intrin.h>
#endif

#if U_PLATFORM_IS_DARWIN_BASED
#if defined(__STRICT_ANSI__)
#define UPRV_REMAP_INLINE
#define inline
#endif
#include <libkern/OSAtomic.h>
#define USE_MAC_OS_ATOMIC_INCREMENT 1
#if defined(UPRV_REMAP_INLINE)
#undef inline
#undef UPRV_REMAP_INLINE
#endif
#endif

/*
 * If we do not compile with dynamic_annotations.h then define
 * empty annotation macros.
 *  See http://code.google.com/p/data-race-test/wiki/DynamicAnnotations
 */
#ifndef ANNOTATE_HAPPENS_BEFORE
# define ANNOTATE_HAPPENS_BEFORE(obj)
# define ANNOTATE_HAPPENS_AFTER(obj)
# define ANNOTATE_UNPROTECTED_READ(x) (x)
#endif

#ifndef UMTX_FULL_BARRIER
# if U_HAVE_GCC_ATOMICS
#  define UMTX_FULL_BARRIER __sync_synchronize();
# elif defined(_MSC_VER) && _MSC_VER >= 1500
    /* From MSVC intrin.h. Use _ReadWriteBarrier() only on MSVC 9 and higher. */
#  define UMTX_FULL_BARRIER _ReadWriteBarrier();
# elif U_PLATFORM_IS_DARWIN_BASED
#  define UMTX_FULL_BARRIER OSMemoryBarrier();
# else
#  define UMTX_FULL_BARRIER \
    { \
        umtx_lock(NULL); \
        umtx_unlock(NULL); \
    }
# endif
#endif

#ifndef UMTX_ACQUIRE_BARRIER
# define UMTX_ACQUIRE_BARRIER UMTX_FULL_BARRIER
#endif

#ifndef UMTX_RELEASE_BARRIER
# define UMTX_RELEASE_BARRIER UMTX_FULL_BARRIER
#endif

/**
 * \def UMTX_CHECK
 * Encapsulates a safe check of an expression
 * for use with double-checked lazy inititialization.
 * Either memory barriers or mutexes are required, to prevent both the hardware
 * and the compiler from reordering operations across the check.
 * The expression must involve only a  _single_ variable, typically
 *    a possibly null pointer or a boolean that indicates whether some service
 *    is initialized or not.
 * The setting of the variable involved in the test must be the last step of
 *    the initialization process.
 *
 * @internal
 */
#define UMTX_CHECK(pMutex, expression, result) \
    { \
        (result)=(expression); \
        UMTX_ACQUIRE_BARRIER; \
    }
/*
 * TODO: Replace all uses of UMTX_CHECK and surrounding code
 * with SimpleSingleton or TriStateSingleton, and remove UMTX_CHECK.
 */

/*
 * Code within ICU that accesses shared static or global data should
 * instantiate a Mutex object while doing so.  The unnamed global mutex
 * is used throughout ICU, so keep locking short and sweet.
 *
 * For example:
 *
 * void Function(int arg1, int arg2)
 * {
 *   static Object* foo;     // Shared read-write object
 *   umtx_lock(NULL);        // Lock the ICU global mutex
 *   foo->Method();
 *   umtx_unlock(NULL);
 * }
 *
 * an alternative C++ mutex API is defined in the file common/mutex.h
 */

/*
 * UMutex - Mutexes for use by ICU implementation code.
 *          Must be declared as static or globals. Can not appear as members
 *          of other objects.
 *          Must be initialized.
 *          Example:
 *            static UMutex = U_MUTEX_INITIALIZER;
 */

#if U_PLATFORM_HAS_WIN32_API
struct UMutex {
  /* TODO */
  CRITICAL_SECTION  cs;
};
#define U_MUTEX_INITIALIZER {}

#elif U_PLATFORM_IMPLEMENTS_POSIX
#include <pthread.h>

struct UMutex {
    pthread_mutex_t  mutex;
};
#define U_MUTEX_INITIALIZER  {PTHREAD_MUTEX_INITIALIZER}

#else
/* Unknow platform type. */
struct UMutex {
    void *fMutex;
};
#define U_MUTEX_INITIALIZER {NULL}
#error Unknown Platform.

#endif

typedef struct UMutex UMutex;
    
/* Lock a mutex.
 * @param mutex The given mutex to be locked.  Pass NULL to specify
 *              the global ICU mutex.  Recursive locks are an error
 *              and may cause a deadlock on some platforms.
 */
U_CAPI void U_EXPORT2 umtx_lock(UMutex* mutex); 

/* Unlock a mutex. Pass in NULL if you want the single global
   mutex. 
 * @param mutex The given mutex to be unlocked.  Pass NULL to specify
 *              the global ICU mutex.
 */
U_CAPI void U_EXPORT2 umtx_unlock (UMutex* mutex);

/*
 * Atomic Increment and Decrement of an int32_t value.
 *
 * Return Values:
 *   If the result of the operation is zero, the return zero.
 *   If the result of the operation is not zero, the sign of returned value
 *      is the same as the sign of the result, but the returned value itself may
 *      be different from the result of the operation.
 */
U_CAPI int32_t U_EXPORT2 umtx_atomic_inc(int32_t *);
U_CAPI int32_t U_EXPORT2 umtx_atomic_dec(int32_t *);

#endif /*_CMUTEX*/
/*eof*/
