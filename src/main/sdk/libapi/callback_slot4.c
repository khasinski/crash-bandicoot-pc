#include "common.h"
/* Forward the caller word through API dispatch slot 04 with index 4. */
s32 _SpuCallback(s32 word) {
    return DMACallback(4,word);
}
