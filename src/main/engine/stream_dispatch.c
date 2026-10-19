#include "common.h"

typedef struct {u16 size,flags;u8 data[1];} Stream;
void * Slst_Update(Stream *source,Stream *reference,Stream *destination,s32 mode) {
    if(!(source->flags&1)) {
        RMemcpy(source->data,destination->data,source->size);
        destination->size=source->size;
        return destination;
    }
    if(mode!=0) {
        Slst_RDecodeForward(source,reference,destination);
    } else {
        Slst_RDecodeBackward(source,reference,destination);
    }
    return destination;
}
