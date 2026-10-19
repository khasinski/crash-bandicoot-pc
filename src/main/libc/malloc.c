#include "common.h"

/*
 * Heap allocator. Each block is preceded by a header word; LIBC_Free marks a
 * block free by setting that word's low bit.
 */

s32 _ExpAllocArea(u32 amount) {
    u32 maximum;
    s32 available, words, next_size;
    if (amount) {
        maximum=amount;
        if (gp_AllocMaxBlockSize>=amount) maximum=gp_AllocMaxBlockSize;
        else amount=gp_AllocMaxBlockSize;
        available=(gp_max_heap-gp_AllocRestBlockTop)*4-8;
        if (available >= (s32)maximum) amount=maximum;
        else if (available < (s32)amount) return -1;
    } else {amount=0;if (gp_AllocRestBlockTop>=gp_max_heap) return -1;}
    words=(s32)amount;
    words=words<0?words+3:words;
    next_size=(s32)(amount+4);
    words >>= 2;
    gp_AllocRestBlockTop[words]=-2;
    *(gp_AllocRestBlockTop - 1) = amount | 1;
    gp_AllocRestBlockTop += next_size/4;
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
                if((u32)((u32 *)gp_max_heap-cursor-4)<extra) break;
                next[1]=extra|1;
                gp_AllocRestBlockTop=(s32 *)(cursor+(extra>>2));
                *gp_AllocRestBlockTop=-2;
                gp_AllocRestBlockTop++;
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

void InitHeap(s32 *base,u32 size) {
    gp_AllocRestBlockTop=base;
    *base=0;
    base += size>>2;
    gp_max_heap=base+1;
    gp_AllocMaxBlockSize=size-4;
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
            *gp_AllocRestBlockTop=-2;
            g_SearchPoint=(u32 *)gp_AllocRestBlockTop;
            gp_AllocRestBlockTop++;
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
    scan=g_TopAllocArea;
    g_SearchPoint=scan;
    goto scan_block;
free_block:
    {
        length=*g_SearchPoint;
        if(tagged<length) {
            u32 *block=g_SearchPoint;
            u32 *tail=(u32 *)(((size/4)*4)+(u32)block);
            *++tail=(length-(u32)size)-4;
            *block=size;
            goto found;
        }
        if(length<(u32)size) {
            next=g_SearchPoint+(length>>2);
            next_length=next[1];
            following=next+1;
            if(next_length==(u32)-2) {
                *g_SearchPoint=next_length;
                gp_AllocRestBlockTop=(s32 *)(g_SearchPoint+1);
            } else if(next_length&1) {
                combined=length+4;
                combined+=next_length&~1;
                *g_SearchPoint=combined;
            } else {
                g_SearchPoint=following;
            }
        }
        goto again;
    }
allocated_block:
    next_length=*g_SearchPoint;
    if(next_length!=(u32)-2) {
        advance=g_SearchPoint+(next_length>>2)+1;
        goto advance_scan;
    }
    passes--;
    if(passes<=0) {
        if(_ExpAllocArea(size)!=0) return NULL;
        goto again;
    }
    advance=g_TopAllocArea;
advance_scan:
    g_SearchPoint=advance;
again:
    scan=g_SearchPoint;
scan_block:
    first_length=*scan;
    if(!(first_length&1)) goto allocated_block;
    tagged=size|1;
    if((u32)first_length!=tagged) goto free_block;
    *scan=first_length&~1;
found:
    return g_SearchPoint+1;
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
        /* This single-iteration scope reproduces the retail register lifetimes.
         * The retail zero-size path frees ptr without defining the result.
         */
        do {
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
            }
        } while (0);
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
