
#include "emacsbuffers.h"

using namespace boost;

namespace {
    static void delBuffer(std::pair<mintcount_t, EmacsBuffer*> buf) {
        delete buf.second;
    } // DelBuffer
};

EmacsBuffers::~EmacsBuffers() {
    std::for_each(_buffers.begin(), _buffers.end(), std::ptr_fun(&delBuffer));
} // ~EmacsBuffers

bool EmacsBuffers::setSearchString(const MintString& str, bool fold_case) {
    regbase::flag_type flags = regbase::literal | (fold_case ? regbase::icase : 0);
    _regex_empty = str.empty();
    if (!_regex_empty) {
        std::string s;
        std::copy(str.begin(), str.end(), std::back_inserter(s));
        _regex.assign(s.c_str(), flags);
    } // if
    return true;
} // setSearchString

bool EmacsBuffers::setSearchRegex(const MintString& exp, bool fold_case) {
#if 1
    regbase::flag_type flags = boost::regex::basic | boost::regex::bk_vbar;
#else
    regbase::flag_type flags = boost::regex::normal | boost::regex::bk_braces
                               | boost::regex::bk_parens | boost::regex::bk_vbar;
#endif
    if (fold_case)
        flags |= regbase::icase;
    _regex_empty = exp.empty();
    if (!_regex_empty) {
        std::string s;
        std::copy(exp.begin(), exp.end(), std::back_inserter(s));
        _regex.assign(s.c_str(), flags);
    } // if
    return true;
} // setSearchRegex

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
    boost::regex_constants::match_flag_type flags = match_not_dot_newline;
    mintcount_t ss_n = std::min(buf.getMarkPosition(ss), buf.size());
    mintcount_t se_n = std::min(buf.getMarkPosition(se), buf.size());
    if (ss_n <= se_n) {
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
    } else {
        EmacsBuffer::const_iterator first = buf.begin() + se_n;
        EmacsBuffer::const_iterator last = buf.end() + ss_n;
        if (first != buf.begin()) {
            // FIXME this is wrong - should be done in the loop
            flags |= match_prev_avail | match_not_bob;
        } // if
        if (last != buf.end()) {
            flags |= match_not_eob;
            if (*last != EmacsBuffer::EOLCHAR) {
                flags |= match_not_eol;
            } // if
        } // if
        for (EmacsBuffer::const_iterator here = last;
             here != first; --here, flags |= match_prev_avail) {
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
    } // else
    return false;
} // search

// EOF
