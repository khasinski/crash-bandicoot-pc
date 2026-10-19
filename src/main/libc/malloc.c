#include "common.h"

/*
 * Heap allocator. Each block is preceded by a header word; LIBC_Free marks a
 * block free by setting that word's low bit.
 */

/* The heap globals hold PS1 addresses of words in game memory. */
#define REST_TOP RAM_AT(s32, gp_AllocRestBlockTop)
#define MAX_HEAP RAM_AT(s32, gp_max_heap)
#define SEARCH RAM_AT(u32, g_SearchPoint)

s32 _ExpAllocArea(u32 amount) {
    u32 maximum;
    s32 available, words, next_size;
    s32 *top;
    if (amount) {
        maximum=amount;
        if (gp_AllocMaxBlockSize>=amount) maximum=gp_AllocMaxBlockSize;
        else amount=gp_AllocMaxBlockSize;
        available=(s32)(MAX_HEAP-REST_TOP)*4-8;
        if (available >= (s32)maximum) amount=maximum;
        else if (available < (s32)amount) return -1;
    } else {amount=0;if (gp_AllocRestBlockTop>=gp_max_heap) return -1;}
    words=(s32)amount;
    words=words<0?words+3:words;
    next_size=(s32)(amount+4);
    words >>= 2;
    top=REST_TOP;
    top[words]=-2;
    *(top - 1) = amount | 1;
    gp_AllocRestBlockTop = Ram_Addr(top + next_size/4);
    return 0;
}

void *_expand(void *ptr,s32 requested) {
    void *result;
    u32 size=((u32)requested+3)&~3;
    u32 *header=(u32 *)ptr-1;
    u32 first_length=*header;
    u32 length;
    u32 *cursor,*next;
    u32 total,extra,next_length,initial;
    s32 *top;
    if(first_length==(u32)-2) return NULL;
    initial=first_length&~1;
    if(initial<size) {
        cursor=header;
        total=initial;
        do {
            next=cursor+(*cursor>>2);
            next_length=next[1];
            cursor=next+1;
            if(next_length!=(u32)-2) {
                if(!(next_length&1)) break;
            } else {
                extra=size-total;
                /* Retail compares available words to a byte count here. */
                if((u32)(RAM_AT(u32, gp_max_heap)-cursor-4)<extra) break;
                next[1]=extra|1;
                top=(s32 *)(cursor+(extra>>2));
                *top=-2;
                gp_AllocRestBlockTop=Ram_Addr(top+1);
            }
            *header+=*cursor&~1;
            total+=*cursor&~1;
        } while(total<size);
    }
    length=*header;
    if(size<(length&~3)) {
        header[(s32)size/4+1]=((length&~1)-size)-4;
        header[(s32)size/4+1]|=1;
        *header=size|(*header&1);
    } else {result=ptr;if((length&~1)<size){ptr=NULL;goto returned;}return result;}
returned:
    return ptr;
}

/* PsyZ declares the libapi prototype; head is a word area in game memory. */
void InitHeap(unsigned long *head,unsigned long size) {
    s32 *base=(s32 *)head;
    u32 bytes=(u32)size;
    gp_AllocRestBlockTop=Ram_Addr(base);
    *base=0;
    base += bytes>>2;
    gp_max_heap=Ram_Addr(base+1);
    gp_AllocMaxBlockSize=bytes-4;
    gp_AllocAreaFlag=0;
}

void *LIBC_Malloc(s32 requested) {
    s32 passes=2;
    s32 size;
    u32 *scan;
    s32 first_length;
    u32 tagged,length,next_length,combined;
    u32 *next,*advance,*following;
    size=requested;size+=3;size&=~3;
    if(gp_AllocAreaFlag==0) {
        if(gp_AllocRestBlockTop<gp_max_heap) {
            *REST_TOP=-2;
            g_SearchPoint=gp_AllocRestBlockTop;
            gp_AllocRestBlockTop+=4;
            if(_ExpAllocArea(size)==0) {
                gp_AllocAreaFlag=1;
                g_TopAllocArea=g_SearchPoint;
                goto start;
            }
            return NULL;
        }
        return NULL;
    }
start:
    scan=RAM_AT(u32, g_TopAllocArea);
    g_SearchPoint=Ram_Addr(scan);
    goto scan_block;
free_block:
    {
        length=*SEARCH;
        if(tagged<length) {
            u32 *block=SEARCH;
            u32 *tail=block+size/4;
            *++tail=(length-(u32)size)-4;
            *block=size;
            goto found;
        }
        if(length<(u32)size) {
            next=SEARCH+(length>>2);
            next_length=next[1];
            following=next+1;
            if(next_length==(u32)-2) {
                *SEARCH=next_length;
                gp_AllocRestBlockTop=g_SearchPoint+4;
            } else if(next_length&1) {
                combined=length+4;
                combined+=next_length&~1;
                *SEARCH=combined;
            } else {
                g_SearchPoint=Ram_Addr(following);
            }
        }
        goto again;
    }
allocated_block:
    next_length=*SEARCH;
    if(next_length!=(u32)-2) {
        advance=SEARCH+(next_length>>2)+1;
        goto advance_scan;
    }
    passes--;
    if(passes<=0) {
        if(_ExpAllocArea(size)!=0) return NULL;
        goto again;
    }
    advance=RAM_AT(u32, g_TopAllocArea);
advance_scan:
    g_SearchPoint=Ram_Addr(advance);
again:
    scan=SEARCH;
scan_block:
    first_length=*scan;
    if(!(first_length&1)) goto allocated_block;
    tagged=size|1;
    if((u32)first_length!=tagged) goto free_block;
    *scan=first_length&~1;
found:
    return SEARCH+1;
}

void LIBC_Free(void *ptr)
{
    ((int *)ptr)[-1] |= 1;
}

void *LIBC_Realloc(void *ptr, int size)
{
    void *res;

    if (!ptr) {
        res = LIBC_Malloc(size);
    } else {
        /* Retail leaves the result undefined after freeing for size 0. */
        if (size) {
            res = _expand(ptr, size);
            if (res) {
                return res;
            }
            res = LIBC_Malloc(size);
            if (res) {
                BIOS_BCopy(ptr, res, size);
                LIBC_Free(ptr);
            }
        } else {
            LIBC_Free(ptr);
            res = NULL;
        }
    }
    return res;
}

void *LIBC_Calloc(int count, int size)
{
    int len;
    void *res;

    len = size * count;
    res = LIBC_Malloc(len);
    if (!res)
        return 0;
    BIOS_BZero(res, len);
    return res;
}
