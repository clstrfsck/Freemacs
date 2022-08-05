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

#include <cctype>

#include "mintstring.h"

namespace {
    mintchar_t digitChar(int n) {
        if (n < 10) {
            return n + '0';
        } else {
            return (n - 10) + 'A';
        } // else
    } // digitChar

    void makeDigits(MintString& s, mintcount_t n, mintcount_t b) {
        int digit = n;
        if (n >= b) {
            digit %= b;
            makeDigits(s, n / b, b);
        } // if
        s.append(1, digitChar(digit));
    } // makeDigits
} // namespace

MintString& stringAppendNum(MintString& s, int n, int b) {
    // Make sure base in range 2..36
    b = std::max(2, std::min(b, 36));
    if (n < 0) {
        s.append(1, '-');
        n = -n;
    } // if
    makeDigits(s, static_cast<mintcount_t>(n), b);
    return s;
} // stringAppendNum

MintString& stringAppendNum(MintString& s, mintcount_t n, int b) {
    // Make sure base in range 2..36
    b = std::max(2, std::min(b, 36));
    makeDigits(s, n, b);
    return s;
} // stringAppendNum

int getStringIntValue(const MintString& s, int base) {
    // Max base uses all digits + letters
    base = std::max(2, std::min(base, 36));
    mintchar_t end_number = '0' + std::min(10, base);
    mintchar_t end_letter = 'A' + std::max(0, base - 10);
    int multval = 1;
    MintString::const_reverse_iterator rbi = s.crbegin();
    MintString::const_reverse_iterator i = rbi;
    while (i != s.crend()) {
        mintchar_t ch = std::toupper(*i);
        if ((ch >= '0' && ch < end_number) || (ch >= 'A' && ch < end_letter)) {
            // This digit is OK, get another one
            ++i;
        } else {
            if (ch == '-') {
                multval = -1;
            } // if
            break;
        } // else
    } // for
    int number = 0;
    if (i != rbi) {
        do {
            --i;
            mintchar_t ch = std::toupper(*i);
            if ((ch >= '0') && (ch < end_number)) {
                int digit = ch - '0';
                number *= base;
                number += digit;
            } else if ((base > 10) && (ch >= 'A' && ch < end_letter)) {
                int digit = 10 + (ch - 'A');
                number *= base;
                number += digit;
            } // else if
        } while (i != rbi);
    } // if
    return number * multval;
} // getStringIntValue

MintString getStringIntPrefix(const MintString& s, int base) {
    // Max base uses all digits + letters
    base = std::max(2, std::min(base, 36));
    mintchar_t end_number = '0' + std::min(10, base);
    mintchar_t end_letter = 'A' + std::max(0, base - 10);
    MintString::const_iterator plast = s.cend();
    while (plast != s.cbegin()) {
        --plast;
        mintchar_t ch = std::toupper(*plast);
        if ((ch >= '0' && ch < end_number) || (ch >= 'A' && ch < end_letter)) {
            // This digit is OK, get another one
        } else {
            // Skip the minus sign if we have one
            if (ch != '-') {
                ++plast;
            } // if
            break;
        } // else
    } // for
    return MintString(s.cbegin(), plast);
} // getStringIntPrefix

// EOF
