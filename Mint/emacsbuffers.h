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

#ifndef _EMACSBUFFERS_H
#define _EMACSBUFFERS_H

#include <map>
#include <memory>
#include <algorithm>

#include "emacsbuffer.h"
#include <boost/regex.hpp>

class EmacsBuffers {
public:

    EmacsBuffers()
        : _regex_empty(true)
    {
        newBuffer();
    } // EmacsBuffers

    // Noncopyable
    EmacsBuffers(EmacsBuffers &) noexcept = delete;
    EmacsBuffers& operator=(const EmacsBuffers &) = delete;
    EmacsBuffers(EmacsBuffers &&) noexcept = delete;
    EmacsBuffers const & operator=(EmacsBuffers &&) = delete;

    EmacsBuffer& getCurBuffer() {
        return *_current_buffer;
    } // GetCurrentBuffer

    const EmacsBuffer& getCurBuffer() const {
        return *_current_buffer;
    } // GetCurrentBuffer

    mintcount_t newBuffer() {
        _current_buffer = std::make_shared<EmacsBuffer>(EmacsBuffer());
        _buffers[_current_buffer->getBufNumber()] = _current_buffer;
        return _current_buffer->getBufNumber();
    } // NewBuffer

    bool selectBuffer(mintcount_t bufno) {
        EmacsBufferMap::iterator i = _buffers.find(bufno);
        if (i != _buffers.end()) {
            _current_buffer = i->second;
            return true;
        } // if
        return false;
    } // SelectBuffer

    bool setSearchString(const MintString& str, bool fold_case);
    bool setSearchRegex(const MintString& exp, bool fold_case);
    bool search(mintchar_t ssmark, mintchar_t semark, mintchar_t msmark, mintchar_t memark);

private:
    std::shared_ptr<EmacsBuffer> _current_buffer;
    typedef std::map<mintcount_t, std::shared_ptr<EmacsBuffer>> EmacsBufferMap;
    EmacsBufferMap _buffers;
    boost::regex _regex;
    bool _regex_empty;

    bool searchForward(EmacsBuffer& buf, mintcount_t ss_n, mintcount_t se_n, mintchar_t ms, mintchar_t me);
    bool searchBackward(EmacsBuffer& buf, mintcount_t ss_n, mintcount_t se_n, mintchar_t ms, mintchar_t me);
}; // EmacsBuffers
    
#endif // _EMACSBUFFERS_H

// EOF
