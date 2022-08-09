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


#include "emacsbuffers.h"

using namespace boost;

bool EmacsBuffers::setSearchString(const MintString& str, bool fold_case) {
    return assignRegex(str, regbase::literal, fold_case);
}

bool EmacsBuffers::setSearchRegex(const MintString& exp, bool fold_case) {
#if 1
    //FIXME: Should use boost:regex::emacs (!) for now.
    regbase::flag_type flags = boost::regex::basic | boost::regex::bk_vbar;
#else
    regbase::flag_type flags = boost::regex::normal | boost::regex::bk_braces
                               | boost::regex::bk_parens | boost::regex::bk_vbar;
#endif
    return assignRegex(exp, flags, fold_case);
}

bool EmacsBuffers::search(mintchar_t ss, mintchar_t se, mintchar_t ms, mintchar_t me) {
    EmacsBuffer& buf = getCurBuffer();
    if (_regex_empty) {
        if (ms) {
            buf.setMark(ms, EmacsBuffer::MARK_POINT);
        } // if
        if (me) {
            buf.setMark(me, EmacsBuffer::MARK_POINT);
        } // if
        return true;
    } // if
    mintcount_t ss_n = std::min(buf.getMarkPosition(ss), buf.size());
    mintcount_t se_n = std::min(buf.getMarkPosition(se), buf.size());
    return (ss_n <= se_n) ? searchForward(buf, ss_n, se_n, ms, me) : searchBackward(buf, ss_n, se_n, ms, me);
}

bool EmacsBuffers::searchForward(EmacsBuffer& buf, mintcount_t ss_n, mintcount_t se_n, mintchar_t ms, mintchar_t me) {
    boost::regex_constants::match_flag_type flags = match_not_dot_newline;
    EmacsBuffer::const_iterator first = buf.begin() + ss_n;
    EmacsBuffer::const_iterator last = buf.begin() + se_n;
    if (first != buf.begin()) {
        flags |= match_prev_avail | match_not_bob;
    } // if
    if (last != buf.end()) {
        flags |= match_not_eob;
        if (*last != EmacsBuffer::EOLCHAR) {
            flags |= match_not_eol;
        } // if
    } // if
    match_results<EmacsBuffer::const_iterator> what;
    if (regex_search(first, last, what, _regex, flags) && what[0].matched) {
        EmacsBuffer::const_iterator first = what[0].first;
        EmacsBuffer::const_iterator last = what[0].second;
        if (ms) {
            buf.setMarkPosition(ms, first - buf.begin());
        } // if
        if (me) {
            buf.setMarkPosition(me, last - buf.begin());
        } // if
        return true;
    } // if
    return false;
}

bool EmacsBuffers::searchBackward(EmacsBuffer& buf, mintcount_t ss_n, mintcount_t se_n, mintchar_t ms, mintchar_t me) {
    boost::regex_constants::match_flag_type flags = match_not_dot_newline;
    EmacsBuffer::const_iterator first = buf.begin() + se_n;
    EmacsBuffer::const_iterator last = buf.end() + ss_n;
    if (last != buf.end()) {
        flags |= match_not_eob;
        if (*last != EmacsBuffer::EOLCHAR) {
            flags |= match_not_eol;
        } // if
    } // if
    boost::regex_constants::match_flag_type first_flags = flags;
    boost::regex_constants::match_flag_type other_flags = flags | match_prev_avail | match_not_bob;
    for (auto here = last; here != first; --here) {
        flags = (first == buf.begin()) ? first_flags : other_flags;
        match_results<EmacsBuffer::const_iterator> what;
        if (regex_match(first, last, what, _regex, flags)) {
            if (ms) {
                buf.setMarkPosition(ms, what[0].first - buf.begin());
            } // if
            if (me) {
                buf.setMarkPosition(me, what[0].second - buf.begin());
            } // if
            return true;
        } // if
    } // for
    return false;
}

bool EmacsBuffers::assignRegex(const MintString &str, regbase::flag_type flags, bool fold_case) {
    bool valid_regex = true;
    if (fold_case) {
        flags |= regbase::icase;
    }
    _regex_empty = str.empty();
    if (!_regex_empty) {
        std::string s(str.cbegin(), str.cend());
        _regex.assign(s.data(), s.size(), flags);
        if (_regex.status()) {
            _regex_empty = true;
            valid_regex = false;
        }
    }
    return valid_regex;
}

// EOF
