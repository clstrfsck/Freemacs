#ifndef _BUFPRIM_H
#define _BUFPRIM_H

#include "mint.h"
#include "emacsbuffer.h"

void registerBufPrims(Mint& interp);

EmacsBuffer& getCurBuffer();

#endif // BUFPRIM_H

// EOF
