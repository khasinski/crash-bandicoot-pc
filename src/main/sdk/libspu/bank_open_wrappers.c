#include "common.h"
s16 SsVabOpenHead(void *data,s16 bank) {return (s16)SsVabOpenHeadWithMode(data,bank,0,0);}
s16 SsVabOpenHeadSticky(void *data,s16 bank,void *buffer) {return (s16)SsVabOpenHeadWithMode(data,bank,1,buffer);}
s16 SsVabFakeHead(void *data,s16 bank,void *buffer) {return (s16)SsVabOpenHeadWithMode(data,bank,1,buffer);}
