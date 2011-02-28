#ifndef _MINTSTRING_H
#define _MINTSTRING_H

#ifdef USE_MINTSTRING_ROPE
# include <ext/rope>
#else
# include <string>
#endif

#include "minttype.h"

#ifdef USE_MINTSTRING_ROPE
typedef __gnu_cxx::crope MintString;
#else
typedef std::string MintString;
# define mutable_end() end()
# define mutable_begin() begin()
#endif

int getStringIntValue(const MintString& s, int base = 10);
MintString getStringIntPrefix(const MintString& s, int base = 10);

MintString& stringAppendNum(MintString& s, int n, int base = 10);
MintString& stringAppendNum(MintString& s, mintcount_t n, int base = 10);

#endif // MINTSTRING_H

// EOF
