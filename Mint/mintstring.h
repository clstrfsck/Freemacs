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

#include <vector>
#include <cstring>
#include <iostream>
#include <iterator>
#include <algorithm>

#include "minttype.h"

class MintString {
private:
    typedef std::vector<mintchar_t> char_vector;
    char_vector _string;

public:
    typedef mintchar_t value_type;
    typedef mintcount_t size_type;
    typedef char_vector::const_iterator const_iterator;
    typedef char_vector::const_reverse_iterator const_reverse_iterator;
    typedef value_type &reference;
    typedef const value_type& const_reference;

    static const size_type npos = static_cast<mintcount_t>(-1);

    MintString() { }
    MintString(mintchar_t ch) : MintString(&ch, 1) { }
    MintString(const mintchar_t *s) : MintString(s, std::strlen(s)) { }
    MintString(const mintchar_t *s, mintcount_t n) : MintString(s, s + n) { }
    template <typename II>
    MintString(II begin, II end) : _string(begin, end) { }

    const_iterator cbegin() const {
        return _string.cbegin();
    }

    const_iterator cend() const {
        return _string.cend();
    }

    const_reverse_iterator crbegin() const {
        return _string.crbegin();
    }

    const_reverse_iterator crend() const {
        return _string.crend();
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
        auto it = std::search(cbegin() + offset, cend(), needle.cbegin(), needle.cend());
        if (it == cend()) {
            return npos;
        }
        return std::distance(cbegin(), it);
    }

    MintString &append(mintchar_t ch) {
        _string.push_back(ch);
        return *this;
    }

    MintString &append(const mintchar_t* s, mintcount_t l) {
        _string.insert(_string.end(), s, s + l);
        return *this;
    }

    MintString &append(const MintString &s) {
        return append(s.cbegin(), s.cend());
    }

    template <typename II>
    MintString &append(II first, II last) {
        _string.insert(_string.end(), first, last);
        return *this;
    }

    MintString &replace(size_type pos, size_type count, mintchar_t ch) {
        if (count == 0) {
            // Insert
            _string.insert(_string.cbegin() + pos, ch);
        } else {
            _string[pos] = ch;
            _string.erase(_string.cbegin() + pos + 1, _string.cbegin() + pos + count);
        }
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
    std::copy(s._string.cbegin(), s._string.cend(), std::ostream_iterator<mintchar_t>(os));
    return os;
}

// Hashing for unordered_map etc
namespace std {
    template <>
    struct hash<MintString> {
        std::size_t operator()(const MintString &str) const {
            const mintchar_t *p = &(str._string[0]);
            std::uint64_t hash = 2166136261UL;
            for (mintcount_t len = str._string.size(); len; --len) {
                hash ^= *p++;
                hash *= static_cast<size_t>(1099511628211ULL);
            }
            return static_cast<std::size_t>(hash);
        }
    };
}

int getStringIntValue(const MintString& s, int base = 10);
MintString getStringIntPrefix(const MintString& s, int base = 10);

MintString& stringAppendNum(MintString& s, int n, int base = 10);
MintString& stringAppendNum(MintString& s, mintcount_t n, int base = 10);

#endif // MINTSTRING_H

// EOF
