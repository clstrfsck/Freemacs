#ifndef _EMACSBUFFER_H
#define _EMACSBUFFER_H

#include <vector>
#ifdef USE_BUFFER_ROPE
# include <ext/rope>
#endif

#include "mintstring.h"


class EmacsBuffer {
private:
#ifdef USE_BUFFER_ROPE
    typedef __gnu_cxx::rope<mintchar_t> BufferString;
#else
    typedef MintString BufferString;
#endif

public:
    typedef BufferString::const_iterator const_iterator;
    typedef BufferString::const_reverse_iterator const_reverse_iterator;

    EmacsBuffer();
    ~EmacsBuffer();

    bool isWriteProtected() const;
    void setWriteProtected(bool iswp);

    bool isModified() const;
    void setModified(bool ismodified);

    bool insertString(const MintString& str);

    bool pushTempMarks(mintcount_t n);
    bool popTempMarks();
    bool createPermMarks(mintcount_t n);

    void setPointToMarks(const MintString& marks);
    void setPointToMark(mintchar_t mark);

    bool setMark(mintchar_t mark, mintchar_t dest_mark);
    bool deleteToMarks(const MintString& marks);
    bool readToMark(mintchar_t mark, MintString* value) const;
    bool readToMark(mintchar_t mark, MintString* value, mintcount_t frompos) const;
    bool translate(mintchar_t mark, const MintString& trstr);
    mintcount_t charsToMark(mintchar_t mark) const;
    bool markBeforePoint(mintchar_t mark) const;
    mintcount_t getBufNumber() const;

    mintcount_t size() const { return _text.size(); }
    const_iterator begin() const { return _text.begin(); }
    const_iterator end() const { return _text.end(); }
    const_reverse_iterator rbegin() const { return _text.rbegin(); }
    const_reverse_iterator rend() const { return _text.rend(); }

    bool setMarkPosition(mintchar_t mark, mintcount_t position);
    mintcount_t getMarkPosition(mintchar_t mark) const;
    mintcount_t getMarkPosition(mintchar_t mark, mintcount_t frompos) const;

    mintcount_t getPointLine() const { return _pointLine; }
    mintcount_t countNewlines() const { return _countNewlines; }
    void setPointLine(mintcount_t lno);

    mintcount_t getColumn() const { return countColumns(getMarkPosition(MARK_BOL), _point); }
    void setColumn(mintcount_t lno);

    mintcount_t countNewlines(mintcount_t from, mintcount_t to) const;
    mintcount_t countColumns(mintcount_t from, mintcount_t to) const;
    mintcount_t getLeftColumn() const           { return _leftcol; }
    void        setTabWidth(mintcount_t n)      { _tab_width = n; }
    mintcount_t getTabWidth() const             { return _tab_width; }
    mintcount_t charWidth(mintcount_t cur_col, mintchar_t ch) const;

    void forcePointInWindow(mintcount_t li, mintcount_t co, mintcount_t tp, mintcount_t bp);

    void        setPointRow(mintcount_t li);
    mintcount_t getPointRow();

    // Temporary marks go from '0'..'9' inclusive
    static const mintchar_t  MARK_FIRST_TEMP  = '0';
    static const mintcount_t MARK_MAX_TEMP    = 10;
    // Permanent marks go from '@'..'Z' inclusive
    static const mintchar_t  MARK_FIRST_PERM  = '@';
    static const mintcount_t MARK_MAX_PERM    = 27;
    // Next and previous char (with respect to point)
    static const mintchar_t  MARK_PREV_CHAR   = '<';
    static const mintchar_t  MARK_NEXT_CHAR   = '>';
    // Beginning and end of file
    static const mintchar_t  MARK_BOB         = '[';
    static const mintchar_t  MARK_EOB         = ']';
    // Beginning and end of line
    static const mintchar_t  MARK_BOL         = '^';
    static const mintchar_t  MARK_EOL         = '$';
    // Next and previous blank character
    static const mintchar_t  MARK_PREV_BLANK  = '-';
    static const mintchar_t  MARK_NEXT_BLANK  = '+';
    // Next and previous non-blank character
    static const mintchar_t  MARK_PREV_NBLANK = '{';
    static const mintchar_t  MARK_NEXT_NBLANK = '}';
    // And, of course, point
    static const mintchar_t  MARK_POINT       = '.';
    // Special window top line marker
    static const mintchar_t  MARK_TOPLINE     = '!';
    // End of line character
    static const mintchar_t  EOLCHAR;

private:
    // Write protected and modified flags
    bool _wp;
    bool _modified;

    // Buffer number
    static mintcount_t _s_bufno;

    // Marks
    mintcount_t _point;
    mintcount_t _topline;
    mintcount_t _leftcol;
    mintcount_t _tab_width;
    mintcount_t _temp_mark_base;
    mintcount_t _temp_mark_last;
    mintcount_t _perm_mark_count;
    mintcount_t _marks_sp;
    static const mintcount_t MAX_MARKS = 50;
    std::vector<mintcount_t> _marks;
    std::vector<mintcount_t> _mark_stack;

    // Count of newlines before point
    mintcount_t _pointLine;
    // Count of newlines before topline
    mintcount_t _toplineLine;
    // Count of newlines in buffer
    mintcount_t _countNewlines;

    mintcount_t _bufno;
    BufferString _text;

    bool deleteToMark(mintchar_t mark);
    mintcount_t forwardLines(mintcount_t pos, mintcount_t lines) const;
    mintcount_t backwardLines(mintcount_t pos, mintcount_t lines) const;

    // Must be called before _point is updated
    void adjustMarksIns(mintcount_t n);
    // Must be called after _point is updated
    void adjustMarksDel(mintcount_t n);

}; // EmacsBuffer
    
#endif // _EMACSBUFFER_H

// EOF
