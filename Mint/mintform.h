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

#ifndef _MINTFORM_H
#define _MINTFORM_H

#include <iostream>
#include <algorithm>

#include "minttype.h"

#ifdef max
#  undef max
#endif
#ifdef min
#  undef min
#endif

class MintForm : public MintString {
public:
    MintForm() : _index(0) { }
    explicit MintForm(const MintString& str) : MintString(str), _index(0) { }
    explicit MintForm(MintString &&str) : MintString(std::move(str)), _index(0) { }

    void setPos(mintcount_t n) {
        _index = std::min(n, size());
    } // setPos

    mintcount_t getPos() const {
        return std::min(_index, size());
    } // getPos

    bool atEnd() const {
        return (_index >= size());
    }

    MintString getN(int n) {
        _index = std::min(_index, size());
        mintcount_t len = std::min(size() - _index, static_cast<mintcount_t>(std::max(0, n)));
        MintString s(cbegin() + _index, cbegin() + _index + len);
        _index += len;
        return s;
    } // getN

    MintString get() {
        _index = std::min(_index, size());
        MintString s(cbegin() + _index, cend());
        return s;
    } // get

private:
    mintcount_t _index;
}; // MintForm

#endif // _MINTFORM_H

// EOF
