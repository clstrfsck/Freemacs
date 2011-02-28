#ifndef _EMACSBUFFERS_H
#define _EMACSBUFFERS_H

#include <map>
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

    virtual ~EmacsBuffers();

    EmacsBuffer& getCurBuffer() {
        return *_current_buffer;
    } // GetCurrentBuffer

    const EmacsBuffer& getCurBuffer() const {
        return *_current_buffer;
    } // GetCurrentBuffer

    mintcount_t newBuffer() {
        _current_buffer = new EmacsBuffer;
        _buffers[_current_buffer->getBufNumber()] = _current_buffer;
        return _current_buffer->getBufNumber();
    } // NewBuffer

    bool selectBuffer(mintcount_t bufno) {
        std::map<mintcount_t, EmacsBuffer*>::iterator i = _buffers.find(bufno);
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
    EmacsBuffer* _current_buffer;
    typedef std::map<mintcount_t, EmacsBuffer*> EmacsBufferMap;
    EmacsBufferMap _buffers;
    boost::regex _regex;
    bool _regex_empty;
}; // EmacsBuffers
    
#endif // _EMACSBUFFERS_H

// EOF
