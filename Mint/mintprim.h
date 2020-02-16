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

#ifndef _MINTPRIM_H
#define _MINTPRIM_H

class Mint;
class MintArgList;

#include "mintstring.h"

class MintPrim {

public:
    virtual ~MintPrim() { }
    virtual void operator()(Mint& interp, bool is_active, const MintArgList& args) = 0;

}; // MintPrim


class MintVar {

public:
    virtual ~MintVar() { }
    virtual MintString getVal(Mint& interp) const = 0;
    virtual void setVal(Mint& interp, const MintString& val) = 0;

}; // MintVar

#endif // _MINTPRIM_H

// EOF
