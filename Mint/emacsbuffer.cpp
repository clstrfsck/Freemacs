#include "emacsbuffer.h"

#include <iostream>
#include <algorithm>
#include <functional>


mintcount_t EmacsBuffer::_s_bufno = 0;
const mintchar_t EmacsBuffer::EOLCHAR = '\n';

EmacsBuffer::EmacsBuffer()
    : _wp(false), _modified(false),
      _point(0),
      _topline(0),
      _leftcol(0),
      _tab_width(8),
      _temp_mark_base(1),
      _temp_mark_last(1),
      _perm_mark_count(1),
      _marks_sp(0),
      _pointLine(0), _toplineLine(0), _countNewlines(0),
      _bufno(++_s_bufno) {
    _marks.resize(MAX_MARKS, 0);
    _mark_stack.resize(MAX_MARKS, 0);
} // EmacsBuffer::EmacsBuffer

EmacsBuffer::~EmacsBuffer() {
} // EmacsBuffer::~EmacsBuffer

bool EmacsBuffer::isWriteProtected() const {
    return _wp;
} // EmacsBuffer::isWriteProtected

void EmacsBuffer::setWriteProtected(bool iswp) {
    _wp = iswp;
} // EmacsBuffer::setWriteProtected

bool EmacsBuffer::isModified() const {
    return _modified;
} // EmacsBuffer::isModified

void EmacsBuffer::setModified(bool ismodified) {
    _modified = ismodified;
} // EmacsBuffer::setModified

bool EmacsBuffer::pushTempMarks(mintcount_t n) {
    if ((_temp_mark_last + n) <= MAX_MARKS) {
        // Link stack
        _mark_stack[_marks_sp++] = _temp_mark_base;
        _temp_mark_base = _temp_mark_last;
        _temp_mark_last = _temp_mark_base + n;
        std::fill_n(_marks.begin() + _temp_mark_base, n, _point);
        return true;
    } // if
    return false;
} // EmacsBuffer::pushTempMarks

bool EmacsBuffer::popTempMarks() {
    if (_marks_sp > 0) {
        _temp_mark_last = _temp_mark_base;
        _temp_mark_base = _mark_stack[--_marks_sp];
        return true;
    } // if
    return false;
} // EmacsBuffer::PopTempMarks

bool EmacsBuffer::createPermMarks(mintcount_t n) {
    if (n <= MARK_MAX_PERM) {
        _perm_mark_count = _temp_mark_base = _temp_mark_last = n;
        _marks_sp = 0;
        return true;
    } // if
    return false;
} // EmacsBuffer::createPermanentMarks

bool EmacsBuffer::setMark(mintchar_t mark, mintchar_t dest_mark) {
    return setMarkPosition(mark, getMarkPosition(dest_mark));
} // EmacsBuffer::SetMark

bool EmacsBuffer::readToMark(mintchar_t mark, MintString* value) const {
    return readToMark(mark, value, _point);
} // EmacsBuffer::readToMark

bool EmacsBuffer::readToMark(mintchar_t mark, MintString* value, mintcount_t frompos) const {
    // This function always returns true for now
    mintcount_t mark_pos = getMarkPosition(mark, frompos);
    mintcount_t min_pos = std::min(_text.size(), std::min(mark_pos, frompos));
    mintcount_t max_pos = std::min(_text.size(), std::max(mark_pos, frompos));
    const_iterator first = _text.begin();
    std::advance(first, min_pos);
    const_iterator last = _text.begin();
    std::advance(last, max_pos);
    std::copy(first, last, std::back_inserter(*value));
    return true;
} // EmacsBuffer::readToMark

mintcount_t EmacsBuffer::charsToMark(mintchar_t mark) const {
    mintcount_t mark_pos = getMarkPosition(mark);
    mintcount_t min_pos = std::min(mark_pos, _point);
    mintcount_t max_pos = std::max(mark_pos, _point);
    return max_pos - min_pos;
} // EmacsBuffer::charsToMark

bool EmacsBuffer::markBeforePoint(mintchar_t mark) const {
    return (getMarkPosition(mark) < _point);
} // EmacsBuffer::markBeforePoint

mintcount_t EmacsBuffer::getBufNumber() const {
    return _bufno;
} // EmacsBuffer::getBufNumber

bool EmacsBuffer::setMarkPosition(mintchar_t mark, mintcount_t position) {
    if (mark >= MARK_FIRST_TEMP) {
        mintcount_t temp_markno = mark - MARK_FIRST_TEMP;
        if ((_temp_mark_base + temp_markno) < _temp_mark_last) {
            _marks[_temp_mark_base + temp_markno] = std::min(_text.size(), position);
            return true;
        } // if
    } // if
    if (mark >= MARK_FIRST_PERM) {
        mintcount_t perm_markno = mark - MARK_FIRST_PERM;
        if (perm_markno < _perm_mark_count) {
            _marks[perm_markno] = std::min(_text.size(), position);
            return true;
        } // if
    } // if
    return false;
} // EmacsBuffer::setMarkPosition


mintcount_t EmacsBuffer::forwardLines(mintcount_t pos, mintcount_t lines) const {
    while (lines--) {
        pos = getMarkPosition(MARK_EOL, pos);
        pos = getMarkPosition(MARK_NEXT_CHAR, pos);
    } // while
    return pos;
} // advanceLines


mintcount_t EmacsBuffer::backwardLines(mintcount_t pos, mintcount_t lines) const {
    while (lines--) {
        pos = getMarkPosition(MARK_PREV_CHAR, pos);
        pos = getMarkPosition(MARK_BOL, pos);
    } // while
    return pos;
} // advanceLines


void EmacsBuffer::forcePointInWindow(mintcount_t li, mintcount_t co,
                                     mintcount_t tp, mintcount_t bp) {
    mintcount_t tl = li * tp / 100;
    if (_pointLine <= tl) {
        // Inside top margin
        _topline = 0;
        _toplineLine = 0;
    } else {
        mintcount_t bl = li * bp / 100;
        if (_pointLine >= _countNewlines - bl) {
            // Inside bottom margin
            _topline = backwardLines(getMarkPosition(MARK_BOL, _text.size()), li - 1);
            _toplineLine = _countNewlines - (li - 1);
        } else {
            if (_pointLine < (_toplineLine + tl)) {
                // Point is before top margin
                mintcount_t blines = (_toplineLine + tl) - _pointLine;
                _topline = backwardLines(_topline, blines);
                _toplineLine -= blines;
            } else if (_pointLine >= (_toplineLine + (li - bl))) {
                // Point is after bottom margin
                mintcount_t flines = _pointLine - (_toplineLine + (li - bl));
                _topline = forwardLines(_topline, flines);
                _toplineLine += flines;
            } // else if
        } // else
    } // else

    // Now the point line is in the window.  Do the same for point column.
    mintcount_t point_column = countColumns(getMarkPosition(MARK_BOL, _point), _point);
    if (point_column < _leftcol) {
        // point is off the left edge of the screen
        _leftcol = (point_column / _tab_width) * _tab_width;
    } else while (point_column >= (_leftcol + co)) {
        // point is off the right edge of the screen
        _leftcol += _tab_width;
    } // else if
} // EmacsBuffer::forcePointInWindow

void EmacsBuffer::setPointRow(mintcount_t li) {
    if (_pointLine <= li) {
        // Not enough lines to have point at row 'li'
        _topline = 0;
        _toplineLine = 0;
    } else {
        _topline = backwardLines(getMarkPosition(MARK_BOL), li);
        _toplineLine = countNewlines(getMarkPosition(MARK_BOB), _topline);
    } // else
} // EmacsBuffer::setPointRow

mintcount_t EmacsBuffer::getPointRow() {
    return countNewlines(getMarkPosition(MARK_TOPLINE), getMarkPosition(MARK_POINT));
} // EmacsBuffer::getPointRow

mintcount_t EmacsBuffer::getMarkPosition(mintchar_t mark) const {
    return getMarkPosition(mark, _point);
} // EmacsBuffer::getMarkPosition

mintcount_t EmacsBuffer::getMarkPosition(mintchar_t mark, mintcount_t frompos) const {
    frompos = std::min(_text.size(), frompos);
    if ((mark >= MARK_FIRST_TEMP) && (mark < static_cast<mintchar_t>(MARK_FIRST_TEMP + MARK_MAX_TEMP))) {
        mintcount_t temp_markno = mark - MARK_FIRST_TEMP;
        if ((_temp_mark_base + temp_markno) < _temp_mark_last) {
            return _marks[_temp_mark_base + temp_markno];
        } // if
    } // if
    if ((mark >= MARK_FIRST_PERM) && (mark < static_cast<mintchar_t>(MARK_FIRST_PERM + MARK_MAX_PERM))) {
        mintcount_t perm_markno = mark - MARK_FIRST_PERM;
        if (perm_markno < _perm_mark_count) {
            return _marks[perm_markno];
        } // if
    } // if
    switch (mark) {
    case MARK_TOPLINE:
        return _topline;
    case MARK_PREV_CHAR:
        return (frompos > 0) ? (frompos - 1) : 0;
    case MARK_NEXT_CHAR:
        return (frompos < _text.size()) ? (frompos + 1) : _text.size();
    case MARK_BOB:
        return 0;
    case MARK_EOB:
        return _text.size();
    case MARK_BOL:
        return findBOL(frompos);
    case MARK_EOL:
        return findEOL(frompos);
    default:
        // Default to returning _point
        return _point;
    } // switch
    // FIXME: The following marks are not yet processed
    // MARK_PREV_BLANK,  MARK_NEXT_BLANK
    // MARK_PREV_NBLANK, MARK_NEXT_NBLANK
} // GetMarkPosition

mintcount_t EmacsBuffer::countNewlines(mintcount_t from, mintcount_t to) const {
    mintcount_t lines = 0;
    if (to > from) {
        lines = std::count(_text.begin() + from, _text.begin() + to, EOLCHAR);
    } // if
    return lines;
} // getLine

void EmacsBuffer::setPointLine(mintcount_t line) {
    // FIXME: This could be optimised to move relative to current
    //        point position if this was closer than starting at top.
    if (line > _pointLine) {
        // Trying to move past end?
        if (line >= _countNewlines) {
            _pointLine = _countNewlines;
            _point = getMarkPosition(MARK_BOL, _text.size());
        } else {
            // We just move forward the specified number of lines
            mintcount_t linediff = line - _pointLine;
            while (linediff--) {
                _point = forwardLines(_point, 1);
                ++_pointLine;
            } // while
        } // else
    } else if (line < _pointLine) {
        // Trying to move to top?
        if (line == 0) {
            _pointLine = 0;
            _point = 0;
        } else {
            // We just move backward the specified number of lines
            mintcount_t linediff = _pointLine - line;
            while (linediff--) {
                _point = backwardLines(_point, 1);
                --_pointLine;
            } // while
        } // else
    } // else if
} // setLine

void EmacsBuffer::setColumn(mintcount_t col) {
    mintcount_t pos = getMarkPosition(MARK_BOL);
    mintcount_t cur_col = 0;
    while ((pos < _text.size()) && (cur_col < col) && (_text[pos] != EOLCHAR)) {
        cur_col += charWidth(cur_col, _text[pos++]);
    } // while
    _point = pos;
} // setColumn

mintcount_t EmacsBuffer::charWidth(mintcount_t cur_col, mintchar_t ch) const {
    if (ch == '\t') {
        return _tab_width - (cur_col % _tab_width);
    } // if
    return 1;
} // charWidth

mintcount_t EmacsBuffer::countColumns(mintcount_t from, mintcount_t to) const {
    mintcount_t cols = 0;
    while (from < to) {
        cols += charWidth(cols, _text[from++]);
    } // while
    return cols;
} // countColumn

void EmacsBuffer::setPointToMarks(const MintString& marks) {
    std::for_each(marks.begin(), marks.end(),
                  std::bind1st(std::mem_fun(&EmacsBuffer::setPointToMark), this));
} // setPointToMarks

void EmacsBuffer::setPointToMark(mintchar_t mark) {
    mintcount_t old_point = _point;
    _point = getMarkPosition(mark);
    // FIXME: This could be optimised by checking distances from BOB/EOB
    //        Also by checking the actual marks used.  Some marks never
    //        change the current line (ie BOL/EOL).
    if (old_point < _point) {
        // Moved forward in the buffer
        _pointLine += countNewlines(old_point, _point);
    } else if (old_point > _point) {
        // Moved backwards in the buffer
        _pointLine -= countNewlines(_point, old_point);
    } // else
} // EmacsBuffer::SetPointToMark


// Buffer mutating functions

bool EmacsBuffer::translate(mintchar_t mark, const MintString& trstr) {
    // This function does not change the number of characters or
    // newlines in the buffer
    mintcount_t p1 = getMarkPosition(MARK_POINT);
    mintcount_t p2 = getMarkPosition(mark);
    if (p1 > p2) {
        std::swap(p1, p2);
    } // if
    if (p1 != p2) {
        MintString translated;
        const_iterator last = _text.begin() + p2;
        for (const_iterator i = _text.begin() + p1; i < last; ++i) {
            umintchar_t ch = static_cast<umintchar_t>(*i);
            if ((ch != EOLCHAR) && (ch < trstr.size())) {
                ch = trstr[ch];
            } // if
            translated.push_back(ch);
        } // for
#if defined(USE_BUFFER_ROPE) && !defined(USE_MINTSTRING_ROPE)
        _text.replace(p1, p2, translated.data(), translated.size());
#else
        _text.replace(p1, p2, translated);
#endif
    } // if
    return true;
} // translate

bool EmacsBuffer::insertString(const MintString& str) {
    if (!str.empty()) {
        _modified = true;
#if defined(USE_BUFFER_ROPE) && !defined(USE_MINTSTRING_ROPE)
        _text.insert(_point, str.data(), str.size());
#else
        _text.insert(_point, str);
#endif
        mintcount_t extra_chars = str.size();
        mintcount_t extra_newlines = countNewlines(_point, _point + extra_chars);
        _countNewlines += extra_newlines;
        _pointLine += extra_newlines;
        if (_topline > _point) {
            // topline moves if it is after point
            _topline += extra_chars;
            _toplineLine += extra_newlines;
        } // if
        adjustMarksIns(extra_chars);
        _point += extra_chars;
    } // if
    return true;
} // EmacsBuffer::insertString

bool EmacsBuffer::deleteToMarks(const MintString& marks) {
    // Stops on first delete that fails
    std::find_if(marks.begin(), marks.end(),
                 std::not1(std::bind1st(std::mem_fun(&EmacsBuffer::deleteToMark), this)));
    return true;
} // EmacsBuffer::DeleteToMarks

bool EmacsBuffer::deleteToMark(mintchar_t mark) {
    mintcount_t mark_pos = getMarkPosition(mark);
    mintcount_t max_pos = std::max(mark_pos, _point);
    mintcount_t min_pos = std::min(mark_pos, _point);
    if (min_pos != max_pos) {
        mintcount_t nchar = max_pos - min_pos;
        if (nchar == _text.size()) {
            // Optimisation for removing whole buffer
            _countNewlines = 0;
            _pointLine = 0;
            _topline = 0;
            _toplineLine = 0;
        } else {
            // Adjust total line count for the lines we are deleting
            mintcount_t allnewlines = countNewlines(min_pos, max_pos);
            _countNewlines -= allnewlines;
            if (max_pos == _point) {
                // Remove any lines prior to point if we are deleting backwards
                _pointLine -= allnewlines;
            } // if
            if (_topline >= max_pos) {
                // Topline is after the deleted text, adjust by amount deleted
                _topline -= nchar;
                _toplineLine -= allnewlines;
            } else if (_topline > min_pos) {
                // If it's in the deleted text, we set it to the BOL near min_pos.
                mintcount_t newpos = getMarkPosition(MARK_BOL, min_pos);
                _toplineLine -= countNewlines(min_pos, _topline);
                _topline = newpos;
            } // else if
        } // else
        _modified = true;
        _text.erase(min_pos, nchar);
        _point = min_pos;
        adjustMarksDel(nchar);
    } // if
    return true;
} // DeleteToMark

// Must be called before _point is updated
void EmacsBuffer::adjustMarksIns(mintcount_t n) {
    for (mintcount_t i = 0; i < _temp_mark_last; ++i) {
        if (_marks[i] >= _point) {
            _marks[i] += n;
        } // if
    } // for
} // AdjustMarksIns

// Must be called after _point is updated
void EmacsBuffer::adjustMarksDel(mintcount_t n) {
    mintcount_t end_ptr = _point + n;
    for (mintcount_t i = 0; i < _temp_mark_last; ++i) {
        if (_marks[i] >= end_ptr) {
            _marks[i] -= n;
        } else if (_marks[i] >= _point) {
            _marks[i] = _point;
        } // else if
    } // for
} // AdjustMarksIns

mintcount_t EmacsBuffer::findBOL(mintcount_t frompos) const {
    mintcount_t bol = frompos;
    while ((bol > 0) && (_text[bol - 1] != EOLCHAR)) {
        bol -= 1;
    } // while
    return bol;
}

mintcount_t EmacsBuffer::findEOL(mintcount_t frompos) const {
    mintcount_t eol = frompos;
    while ((eol < _text.size()) && (_text[eol] != EOLCHAR)) {
        eol += 1;
    } // while
    return eol;
}

// EOF
