/*
*******************************************************************************
*   Copyright (C) 2010, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*   file name:  uchartrie.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2010nov14
*   created by: Markus W. Scherer
*/

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "uassert.h"
#include "uchartrie.h"

U_NAMESPACE_BEGIN

Appendable &
Appendable::appendCodePoint(UChar32 c) {
    if(c<=0xffff) {
        return append((UChar)c);
    } else {
        return append(U16_LEAD(c)).append(U16_TRAIL(c));
    }
}

Appendable &
Appendable::append(const UChar *s, int32_t length) {
    if(s!=NULL && length!=0) {
        if(length<0) {
            UChar c;
            while((c=*s++)!=0) {
                append(c);
            }
        } else {
            const UChar *limit=s+length;
            while(s<limit) {
                append(*s++);
            }
        }
    }
    return *this;
}

UOBJECT_DEFINE_NO_RTTI_IMPLEMENTATION(Appendable)

UDictTrieResult
UCharTrie::current() const {
    const UChar *pos=pos_;
    if(pos==NULL) {
        return UDICTTRIE_NO_MATCH;
    } else {
        int32_t node;
        return (remainingMatchLength_<0 && (node=*pos)>=kMinValueLead) ?
                valueResult(node) : UDICTTRIE_NO_VALUE;
    }
}

UDictTrieResult
UCharTrie::branchNext(const UChar *pos, int32_t length, int32_t uchar) {
    // Branch according to the current unit.
    if(length==0) {
        length=*pos++;
    }
    ++length;
    // The length of the branch is the number of units to select from.
    // The data structure encodes a binary search.
    while(length>kMaxBranchLinearSubNodeLength) {
        if(uchar<*pos++) {
            length>>=1;
            pos=jumpByDelta(pos);
        } else {
            length=length-(length>>1);
            pos=skipDelta(pos);
        }
    }
    // Drop down to linear search for the last few units.
    // length>=2 because the loop body above sees length>kMaxBranchLinearSubNodeLength>=3
    // and divides length by 2.
    do {
        if(uchar==*pos++) {
            UDictTrieResult result;
            int32_t node=*pos;
            if(node&kValueIsFinal) {
                // Leave the final value for getValue() to read.
                result=UDICTTRIE_HAS_FINAL_VALUE;
            } else {
                // Use the non-final value as the jump delta.
                ++pos;
                // int32_t delta=readValue(pos, node>>1);
                int32_t delta;
                if(node<kMinTwoUnitValueLead) {
                    delta=node;
                } else if(node<kThreeUnitValueLead) {
                    delta=((node-kMinTwoUnitValueLead)<<16)|*pos++;
                } else {
                    delta=(pos[0]<<16)|pos[1];
                    pos+=2;
                }
                // end readValue()
                pos+=delta;
                node=*pos;
                result= node>=kMinValueLead ? valueResult(node) : UDICTTRIE_NO_VALUE;
            }
            pos_=pos;
            return result;
        }
        --length;
        pos=skipValue(pos);
    } while(length>1);
    if(uchar==*pos++) {
        pos_=pos;
        int32_t node=*pos;
        return node>=kMinValueLead ? valueResult(node) : UDICTTRIE_NO_VALUE;
    } else {
        stop();
        return UDICTTRIE_NO_MATCH;
    }
}

UDictTrieResult
UCharTrie::nextImpl(const UChar *pos, int32_t uchar) {
    int32_t node=*pos++;
    for(;;) {
        if(node<kMinLinearMatch) {
            return branchNext(pos, node, uchar);
        } else if(node<kMinValueLead) {
            // Match the first of length+1 units.
            int32_t length=node-kMinLinearMatch;  // Actual match length minus 1.
            if(uchar==*pos++) {
                remainingMatchLength_=--length;
                pos_=pos;
                return (length<0 && (node=*pos)>=kMinValueLead) ?
                        valueResult(node) : UDICTTRIE_NO_VALUE;
            } else {
                // No match.
                break;
            }
        } else if(node&kValueIsFinal) {
            // No further matching units.
            break;
        } else {
            // Skip intermediate value.
            pos=skipNodeValue(pos, node);
            node&=kNodeTypeMask;
        }
    }
    stop();
    return UDICTTRIE_NO_MATCH;
}

UDictTrieResult
UCharTrie::next(int32_t uchar) {
    const UChar *pos=pos_;
    if(pos==NULL) {
        return UDICTTRIE_NO_MATCH;
    }
    int32_t length=remainingMatchLength_;  // Actual remaining match length minus 1.
    if(length>=0) {
        // Remaining part of a linear-match node.
        if(uchar==*pos++) {
            remainingMatchLength_=--length;
            pos_=pos;
            int32_t node;
            return (length<0 && (node=*pos)>=kMinValueLead) ?
                    valueResult(node) : UDICTTRIE_NO_VALUE;
        } else {
            stop();
            return UDICTTRIE_NO_MATCH;
        }
    }
    return nextImpl(pos, uchar);
}

UDictTrieResult
UCharTrie::next(const UChar *s, int32_t sLength) {
    if(sLength<0 ? *s==0 : sLength==0) {
        // Empty input.
        return current();
    }
    const UChar *pos=pos_;
    if(pos==NULL) {
        return UDICTTRIE_NO_MATCH;
    }
    int32_t length=remainingMatchLength_;  // Actual remaining match length minus 1.
    for(;;) {
        // Fetch the next input unit, if there is one.
        // Continue a linear-match node without rechecking sLength<0.
        int32_t uchar;
        if(sLength<0) {
            for(;;) {
                if((uchar=*s++)==0) {
                    remainingMatchLength_=length;
                    pos_=pos;
                    int32_t node;
                    return (length<0 && (node=*pos)>=kMinValueLead) ?
                            valueResult(node) : UDICTTRIE_NO_VALUE;
                }
                if(length<0) {
                    remainingMatchLength_=length;
                    break;
                }
                if(uchar!=*pos) {
                    stop();
                    return UDICTTRIE_NO_MATCH;
                }
                ++pos;
                --length;
            }
        } else {
            for(;;) {
                if(sLength==0) {
                    remainingMatchLength_=length;
                    pos_=pos;
                    int32_t node;
                    return (length<0 && (node=*pos)>=kMinValueLead) ?
                            valueResult(node) : UDICTTRIE_NO_VALUE;
                }
                uchar=*s++;
                --sLength;
                if(length<0) {
                    remainingMatchLength_=length;
                    break;
                }
                if(uchar!=*pos) {
                    stop();
                    return UDICTTRIE_NO_MATCH;
                }
                ++pos;
                --length;
            }
        }
        int32_t node=*pos++;
        for(;;) {
            if(node<kMinLinearMatch) {
                UDictTrieResult result=branchNext(pos, node, uchar);
                if(result==UDICTTRIE_NO_MATCH) {
                    return UDICTTRIE_NO_MATCH;
                }
                // Fetch the next input unit, if there is one.
                if(sLength<0) {
                    if((uchar=*s++)==0) {
                        return result;
                    }
                } else {
                    if(sLength==0) {
                        return result;
                    }
                    uchar=*s++;
                    --sLength;
                }
                if(result==UDICTTRIE_HAS_FINAL_VALUE) {
                    // No further matching units.
                    stop();
                    return UDICTTRIE_NO_MATCH;
                }
                pos=pos_;  // branchNext() advanced pos and wrote it to pos_ .
                node=*pos++;
            } else if(node<kMinValueLead) {
                // Match length+1 units.
                length=node-kMinLinearMatch;  // Actual match length minus 1.
                if(uchar!=*pos) {
                    stop();
                    return UDICTTRIE_NO_MATCH;
                }
                ++pos;
                --length;
                break;
            } else if(node&kValueIsFinal) {
                // No further matching units.
                stop();
                return UDICTTRIE_NO_MATCH;
            } else {
                // Skip intermediate value.
                pos=skipNodeValue(pos, node);
                node&=kNodeTypeMask;
            }
        }
    }
}

const UChar *
UCharTrie::findUniqueValueFromBranch(const UChar *pos, int32_t length,
                                    UBool haveUniqueValue, int32_t &uniqueValue) {
    while(length>kMaxBranchLinearSubNodeLength) {
        ++pos;  // ignore the comparison byte
        if(NULL==findUniqueValueFromBranch(jumpByDelta(pos), length>>1, haveUniqueValue, uniqueValue)) {
            return NULL;
        }
        length=length-(length>>1);
        pos=skipDelta(pos);
    }
    do {
        ++pos;  // ignore a comparison unit
        // handle its value
        int32_t node=*pos++;
        UBool isFinal=(UBool)(node>>15);
        node&=0x7fff;
        int32_t value=readValue(pos, node);
        pos=skipValue(pos, node);
        if(isFinal) {
            if(haveUniqueValue) {
                if(value!=uniqueValue) {
                    return NULL;
                }
            } else {
                uniqueValue=value;
                haveUniqueValue=TRUE;
            }
        } else {
            if(!findUniqueValue(pos+value, haveUniqueValue, uniqueValue)) {
                return NULL;
            }
            haveUniqueValue=TRUE;
        }
    } while(--length>1);
    return pos+1;  // ignore the last comparison unit
}

UBool
UCharTrie::findUniqueValue(const UChar *pos, UBool haveUniqueValue, int32_t &uniqueValue) {
    int32_t node=*pos++;
    for(;;) {
        if(node<kMinLinearMatch) {
            if(node==0) {
                node=*pos++;
            }
            pos=findUniqueValueFromBranch(pos, node+1, haveUniqueValue, uniqueValue);
            if(pos==NULL) {
                return FALSE;
            }
            haveUniqueValue=TRUE;
            node=*pos++;
        } else if(node<kMinValueLead) {
            // linear-match node
            pos+=node-kMinLinearMatch+1;  // Ignore the match units.
            node=*pos++;
        } else {
            UBool isFinal=(UBool)(node>>15);
            int32_t value;
            if(isFinal) {
                value=readValue(pos, node&0x7fff);
            } else {
                value=readNodeValue(pos, node);
            }
            if(haveUniqueValue) {
                if(value!=uniqueValue) {
                    return FALSE;
                }
            } else {
                uniqueValue=value;
                haveUniqueValue=TRUE;
            }
            if(isFinal) {
                return TRUE;
            }
            pos=skipNodeValue(pos, node);
            node&=kNodeTypeMask;
        }
    }
}

int32_t
UCharTrie::getNextUChars(Appendable &out) const {
    const UChar *pos=pos_;
    if(pos==NULL) {
        return 0;
    }
    if(remainingMatchLength_>=0) {
        out.append(*pos);  // Next unit of a pending linear-match node.
        return 1;
    }
    int32_t node=*pos++;
    if(node>=kMinValueLead) {
        if(node&kValueIsFinal) {
            return 0;
        } else {
            pos=skipNodeValue(pos, node);
            node&=kNodeTypeMask;
        }
    }
    if(node<kMinLinearMatch) {
        if(node==0) {
            node=*pos++;
        }
        getNextBranchUChars(pos, ++node, out);
        return node;
    } else {
        // First unit of the linear-match node.
        out.append(*pos);
        return 1;
    }
}

void
UCharTrie::getNextBranchUChars(const UChar *pos, int32_t length, Appendable &out) {
    while(length>kMaxBranchLinearSubNodeLength) {
        ++pos;  // ignore the comparison unit
        getNextBranchUChars(jumpByDelta(pos), length>>1, out);
        length=length-(length>>1);
        pos=skipDelta(pos);
    }
    do {
        out.append(*pos++);
        pos=skipValue(pos);
    } while(--length>1);
    out.append(*pos);
}

U_NAMESPACE_END