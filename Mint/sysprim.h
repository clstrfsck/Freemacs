#ifndef _SYSPRIM_H
#define _SYSPRIM_H

#include "mint.h"

void registerSysPrims(Mint& interp);

extern int global_argc;
extern char **global_argv;
extern char **global_envp;

#endif // _SYSPRIM_H

// EOF
