/*
 * Copyright 2020 Martin Sandiford
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to: Free Software Foundation
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

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
