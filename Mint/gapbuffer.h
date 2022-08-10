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

#ifndef _GAPBUFFER_H
#define _GAPBUFFER_H

#include <utility>
#include <vector>

#include "mintstring.h"
#include "minttype.h"

#include <iostream>

class GapBuffer {
public:
    class GapBufferIterator {
    public:
        GapBufferIterator() : _buffer(0), _offset(0) {}

        GapBufferIterator(const GapBufferIterator &other) = default;

        GapBufferIterator &operator=(GapBufferIterator other) {
            std::swap(_buffer, other._buffer);
            std::swap(_offset, other._offset);
            return *this;
        }

        bool operator>(const GapBufferIterator &rhs) const {
            return _offset > rhs._offset;
        }

        bool operator<(const GapBufferIterator &rhs) const {
            return _offset < rhs._offset;
        }

        bool operator==(const GapBufferIterator &rhs) const {
            return _offset == rhs._offset;
        }

        bool operator!=(const GapBufferIterator &rhs) const {
            return _offset != rhs._offset;
        }

        GapBufferIterator operator++() {
            if (_offset < _buffer->size()) {
                _offset += 1;
            }
            return *this;
        }

        GapBufferIterator operator++(int) {
            GapBufferIterator temp{*this};
            this->operator++();
            return temp;
        }

        GapBufferIterator operator--() {
            if (_offset > 0) {
                _offset -= 1;
            }
            return *this;
        }

        GapBufferIterator operator--(int) {
            GapBufferIterator temp{*this};
            this->operator--();
            return temp;
        }

        mintchar_t operator*() const {
            return (*_buffer)[_offset];
        }

        mintcount_t operator-(const GapBufferIterator rhs) const {
            return _offset - rhs._offset;
        }

        GapBufferIterator &operator+=(mintcount_t n) {
            _offset += n;
            return *this;
        }

        GapBufferIterator &operator-=(mintcount_t n) {
            _offset -= n;
            return *this;
        }

        friend GapBufferIterator operator+(GapBufferIterator lhs, mintcount_t n) {
            lhs += n;
            return lhs;
        }

        friend GapBufferIterator operator-(GapBufferIterator lhs, mintcount_t n) {
            lhs -= n;
            return lhs;
        }

    private:
        GapBufferIterator(const GapBuffer *buffer, mintcount_t offset)
            : _buffer(buffer), _offset(offset) {}

        const GapBuffer *_buffer;
        mintcount_t _offset;
        friend class GapBuffer;
    };

    typedef GapBufferIterator const_iterator;

    static const mintcount_t BLOCK_SIZE = 65536;

    // Constructors and initial set up
    explicit GapBuffer(mintcount_t size = BLOCK_SIZE)
        : _toptop(0), _bottop(0), _topbot(size), _botbot(size), _buffer(size) {}

    mintcount_t free() const {
        return _topbot - _bottop;
    }
    
    mintcount_t allocated() const {
        return _botbot - _toptop;
    }

    mintcount_t size() const {
        return allocated() - free();
    }

    mintchar_t operator[](mintcount_t offset) const {
        if (offset >= _bottop) {
            // Skip over gap
            offset += free();
        }
        return _buffer[offset];
    }

    const_iterator begin() const {
        return GapBufferIterator(this, 0);
    }
    const_iterator end() const {
        return GapBufferIterator(this, size());
    }

    // Interface
    bool replace(mintcount_t offset, mintcount_t n, const MintString &replacement) {
        return erase(offset, n) && insert(offset, replacement);
    }

    bool erase(mintcount_t offset, mintcount_t n) {
        if ((size() - offset) >= n && move_gap_to(offset + n)) {
            _bottop -= n;
            return true;
        }
        return false;
    }

    bool insert(mintcount_t offset, const MintString &to_insert) {
        auto insert_size = to_insert.size();
        if (free() < insert_size) {
            expand(insert_size - free());
        }
        if (free() >= insert_size && move_gap_to(offset)) {
            std::copy(to_insert.cbegin(), to_insert.cend(), _buffer.begin() + _bottop);
            _bottop += insert_size;
            return true;
        }
        return false;
    }

private:
    bool move_gap_to(mintcount_t offset) {
        if (offset == _bottop) {
            // Fast path: gap where we want it
            return true;
        }
        if (offset > size()) {
            // Can't move past end of buffer, soz.
            return false;
        }
        if (offset < _bottop) {
            // Moving back into top segment
            mintcount_t move_size = _bottop - offset;
            auto begin = _buffer.begin() + offset;
            auto end = begin + move_size;
            auto dest = _buffer.begin() + (_topbot - move_size);
            std::copy(begin, end, dest);
            _bottop -= move_size;
            _topbot -= move_size;
        } else {
            // Moving into bottom segment
            mintcount_t move_size = offset - _bottop;
            auto begin = _buffer.begin() + _topbot;
            auto end = begin + move_size;
            auto dest = _buffer.begin() + offset;
            std::copy_backward(begin, end, dest);
            _bottop += move_size;
            _topbot += move_size;
        }
        return true;
    }

    void expand(mintcount_t extra_space) {
        if (extra_space) {
            mintcount_t additional_blocks = (extra_space - free() + BLOCK_SIZE) / BLOCK_SIZE;
            mintcount_t new_size = allocated() + additional_blocks * BLOCK_SIZE;
            if (new_size > allocated()) { // Make sure we don't overflow
                move_gap_to(size());
                _buffer.resize(new_size);
                _botbot = _topbot = _buffer.size();
            }
        }
    }

    // Note that in this implementation, _toptop = 0 and _botbot = _buffer.size()
    // always
    mintcount_t _toptop;
    mintcount_t _bottop; // Invariant: _toptop <= _bottop
    mintcount_t _topbot; // Invariant: _bottop <= _topbot
    mintcount_t _botbot; // Invariant: _topbot <= _botbot
    std::vector<mintchar_t> _buffer;
};

template <> struct std::iterator_traits<GapBuffer::GapBufferIterator> {
    typedef mintcount_t difference_type;
    typedef mintchar_t value_type;
    typedef const mintchar_t &reference;
    typedef const mintchar_t *pointer;
    typedef std::random_access_iterator_tag iterator_category;
};

#endif // _GAPBUFFER_H

// EOF
