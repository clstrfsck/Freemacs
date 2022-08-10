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

#include <string>
#include <iostream>

#include "minttype.h"

class MintString {
private:
    std::string _string;

public:
    typedef std::string::value_type value_type;
    typedef mintcount_t size_type;
    typedef std::string::const_iterator const_iterator;
    typedef std::string::const_reverse_iterator const_reverse_iterator;
    typedef value_type &reference;
    typedef const value_type& const_reference;

    static const size_type npos = static_cast<mintcount_t>(-1);

    MintString() { }
    MintString(mintchar_t ch) : _string(1, ch) { }
    MintString(const mintchar_t *s) : _string(s) { }
    MintString(const mintchar_t *s, mintcount_t n) : _string(s, n) { }
    template <typename II>
    MintString(II begin, II end) : _string(begin, end) { }

    const_iterator cbegin() const {
        return _string.begin();
    }

    const_iterator cend() const {
        return _string.end();
    }

    const_reverse_iterator crbegin() const {
        return _string.rbegin();
    }

    const_reverse_iterator crend() const {
        return _string.rend();
    }

    value_type operator[](size_type index) const {
        return _string[index];
    }

    bool empty() const {
        return _string.empty();
    }

    size_type size() const {
        return _string.size();
    }

    size_type find(const MintString &needle, size_type offset = 0) const {
        return _string.find(needle._string.c_str(), offset);
    }

    MintString &append(mintchar_t ch) {
        _string.push_back(ch);
        return *this;
    }

    MintString &append(const mintchar_t* s, mintcount_t l) {
        _string.append(s, l);
        return *this;
    }

    MintString &append(const MintString &s) {
        _string.append(s._string);
        return *this;
    }

    template <typename II>
    MintString &append(II first, II last) {
        _string.append(first, last);
        return *this;
    }

    MintString &replace(size_type pos, size_type count, mintchar_t ch) {
        _string.replace(pos, count, 1, ch);
        return *this;
    }

    void clear() {
        _string.clear();
    }

    bool operator<(const MintString& arg) const {
        return _string < arg._string;
    }

    bool operator<=(const MintString& arg) const {
        return _string <= arg._string;
    }

    bool operator>(const MintString& arg) const {
        return _string > arg._string;
    }

    bool operator>=(const MintString& arg) const {
        return _string >= arg._string;
    }

    bool operator==(const MintString& arg) const {
        return _string == arg._string;
    }

    bool operator!=(const MintString& arg) const {
        return _string != arg._string;
    }

    friend std::ostream& operator<<(std::ostream& os, const MintString &s);
    friend struct std::hash<MintString>;
};

inline std::ostream& operator<<(std::ostream& os, const MintString &s) {
    os << s._string;
    return os;
}

// Hashing for unordered_map etc
namespace std {
    template <>
    struct hash<MintString> {
        std::size_t operator()(const MintString &s) const {
            return hash<std::string>()(s._string);
        }
    };
}

int getStringIntValue(const MintString& s, int base = 10);
MintString getStringIntPrefix(const MintString& s, int base = 10);

MintString& stringAppendNum(MintString& s, int n, int base = 10);
MintString& stringAppendNum(MintString& s, mintcount_t n, int base = 10);

#endif // MINTSTRING_H

// EOF
