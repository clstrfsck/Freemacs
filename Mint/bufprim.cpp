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

#include <memory>
#include <fstream>
#include <iterator>

#include <errno.h>
#include <string.h>

#include "bufprim.h"
#include "emacsbuffers.h"

namespace {
    EmacsBuffers mint_buffers;
} // namespace

// #(ba,X,Y)
// ---------
// Buffer allocate/select.  "X" is interpreted as a decimal number.  If "X"
// is less than zero, the current buffer number is returned.  If "X" equals
// zero, then a new buffer is created, and its buffer number returned.  If
// "X" is greater than zero, that buffer is selected and its number
// returned if it exists, otherwise zero is returned.  If an existing
// buffer is selected, and "Y" is non-null, the buffer is selected without
// necessarily expanding its size, which is cheap and means that the buffer
// cannot be modified.
//
// Returns: The buffer number of the current/selected/created buffer, or
// zero if no such buffer exists.
class baPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        // Second parameter is ignored for now, as buffers are not stored packed
        auto argi = args.cbegin();
        auto whattodo = args.nextArg(argi).getIntValue();
        if (whattodo == 0) {
            mintcount_t buf_num = mint_buffers.newBuffer();
            interp.returnInteger(is_active, buf_num);
        } else if (whattodo < 0) {
            mintcount_t buf_num = mint_buffers.getCurBuffer().getBufNumber();
            interp.returnInteger(is_active, buf_num);
        } else {
            // whattodo must be greater than zero
            mintcount_t buf_num = (mint_buffers.selectBuffer(whattodo)
                                   ? mint_buffers.getCurBuffer().getBufNumber() : 0);
            interp.returnInteger(is_active, buf_num);
        } // else
    } // operator()
}; // baPrim

// #(is,X,Y)
// ---------
// Insert string.  Inserts string "X" into the current buffer.
//
// Returns: Returns "Y" if inserted OK, null otherwise.
class isPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &string = args.nextArg(argi).getValue();
        if (mint_buffers.getCurBuffer().insertString(string)) {
            auto retval = args.nextArg(argi).getValue();
            interp.returnString(is_active, retval);
        } else {
            interp.returnNull(is_active);
        } // else
    } // operator()
}; // isPrim

// #(pm,X,Y)
// -------
// Push/pop mark.  If "X" is greater than zero, that many temporary marks
// are stacked.  If "X" is less than zero, the absolute value of that many
// permanent marks are stacked.  If "X" is zero, temporary marks are
// unstacked.  All newly stacked marks are set to the current value of
// point.
//
// Returns: null if successful, "Y" in active mode if an error occurs (ie
// "X" would case the mark stack to overflow).
class pmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto whattodo = args.nextArg(argi).getIntValue();
        bool ok = false;
        if (whattodo > 0) {
            ok = mint_buffers.getCurBuffer().pushTempMarks(whattodo);
        } else if (whattodo == 0) {
            ok = mint_buffers.getCurBuffer().popTempMarks();
        } else {
            ok = mint_buffers.getCurBuffer().createPermMarks(-whattodo);
        } // else
        if (ok) {
            interp.returnNull(is_active);
        } else {
            auto &errorString = args.nextArg(argi).getValue();
            interp.returnString(true, errorString);
        } // else
    } // operator()
}; // pmPrim

// #(sm,X,Y)
// ---------
// Set mark.  Set user mark "X" to mark "Y".  If mark "Y" is not specified
// or null, then "." is used.  The following values are acceptable for "Y":
//     '0..9'  User temporary marks
//     '@..Z'  User permanent marks
//     '*'     If the current buffer is displayed in both windows, this is
//             the value of point in the other window
//     '>'     Character to the right of the point
//     '<'     Character to the left of the point
//     '['     First character in the file
//     ']'     Last character in the file
//     '^'     Beginning of the current line
//     '$'     End of the current line
//     '-'     First blank char to the left
//     '+'     First blank char to the right
//     '{'     First non-blank char to the left
//     '}'     First non-blank char to the right
//     '.'     Point
// The following are valid values for "X":
//     '0..9'  User temporary marks
//     '@..Z'  User permanent marks
//     '*'     If the current buffer is displayed in both windows, this is
//             the value of point in the other window
//
// Returns: null
class smPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &userMark = args.nextArg(argi).getValue();
        if (!userMark.empty()) {
            auto &mark = args.nextArg(argi).getValue();
            mint_buffers.getCurBuffer().setMark(userMark[0], mark.empty() ? '.' : mark[0]);
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // smPrim

// #(sp,X)
// -------
// Set point.  Sets point to mark given by "X".  See #(sm,...) for details
// of valid values for "X".
//
// Returns: null
class spPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &marks = args.nextArg(argi).getValue();
        mint_buffers.getCurBuffer().setPointToMarks(marks);
        interp.returnNull(is_active);
    } // operator()
}; // spPrim

// #(dm,X)
// -------
// Delete to mark.  Delete from point to marks specified in string "X".
//
// Returns: null
class dmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &marks = args.nextArg(argi).getValue();
        mint_buffers.getCurBuffer().deleteToMarks(marks);
        interp.returnNull(is_active);
    } // operator()
}; // dmPrim

// #(rm,X,Y)
// -------
// Read to mark.  Read from point to mark "X".  If there is insufficient
// space for the string, "Y" is returned in active mode.
//
// Returns: The buffer between point and mark "X" if enough space exists,
// otherwise return "Y" in active mode.
class rmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString s;
        auto argi = args.cbegin();
        auto &mark = args.nextArg(argi).getValue();
        if (!mark.empty() && mint_buffers.getCurBuffer().readToMark(mark[0], &s)) {
            interp.returnString(is_active, s);
        } else {
            auto &errorString = args.nextArg(argi).getValue();
            interp.returnString(true, errorString);
        } // else
    } // operator()
}; // rmPrim

// #(rc,X)
// -------
// Read count.  Read count of characters between point and mark "X".
//
// Returns: The number of characters between point and mark as a decimal
// number.
class rcPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &mark = args.nextArg(argi).getValue();
        auto count = !mark.empty() ? mint_buffers.getCurBuffer().charsToMark(mark[0]) : 0;
        interp.returnInteger(is_active, count);
    } // operator()
}; // rcPrim

// #(mb,X,A,B)
// -----------
// Mark before.
//
// Returns: "A" if mark "X" is before point, "B" otherwise.
class mbPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &mark = args.nextArg(argi).getValue();
        auto &arg_a = args.nextArg(argi).getValue();
        if (!mark.empty() && mint_buffers.getCurBuffer().markBeforePoint(mark[0])) {
            interp.returnString(is_active, arg_a);
        } else {
            auto &arg_b = args.nextArg(argi).getValue();
            interp.returnString(is_active, arg_b);
        } // else
    } // operator()
}; // mbPrim

// #(rf,X)
// -------
// Read file.  File given by literal string "X" is read into current
// buffer.
//
// Returns: null if successful, otherwise returns error message string.
class rfPrim : public MintPrim {
    static const mintcount_t BUF_SIZE = 4096;
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        auto argi = args.cbegin();
        auto &file_name = args.nextArg(argi).getValue();
        std::string fn(file_name.cbegin(), file_name.cend());
#ifdef _VERBOSE_DEBUG
        std::cerr << "rfPrim: Reading file " << fn << std::endl;
#endif
        std::ifstream in(fn.c_str(), std::ios::binary);
        if (in.good()) {
            errno = 0;
            std::vector<mintchar_t> buf;
            buf.resize(BUF_SIZE);
            while (!in.eof()) {
                mintcount_t read_chars =
                    in.read(reinterpret_cast<char *>(&(buf[0])), BUF_SIZE).gcount();
                if (read_chars == 0)
                    break;
                MintString inserted_chars(&(buf[0]), read_chars);
                mint_buffers.getCurBuffer().insertString(inserted_chars);
            } // while
            in.close();
        } else {
#ifdef _VERBOSE_DEBUG
            std::cerr << "rfPrim: Error opening file " << fn << ": " << strerror(errno) << std::endl;
            interp.print();
#endif
            ret = strerror(errno);
        } // else
        interp.returnString(is_active, ret);
    } // operator()
}; // rfPrim

// #(wf,X,Y)
// ---------
// Write file.  Write text between point and mark "Y" to file given by
// literal string "X".
//
// Returns: null if write is successful, otherwise error message string.
class wfPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        auto argi = args.cbegin();
        auto &file_name = args.nextArg(argi).getValue();
        std::string fn(file_name.cbegin(), file_name.cend());
#ifdef _VERBOSE_DEBUG
        std::cerr << "wfPrim: Writing file " << fn << std::endl;
#endif
        std::ofstream out(fn.c_str(), std::ios::binary);
        if (out.good()) {
            auto &mark_string = args.nextArg(argi).getValue();
            mintchar_t mark = mark_string.empty() ? '.' : mark_string[0];
            MintString text;
            if (mint_buffers.getCurBuffer().readToMark(mark, &text)) {
                std::copy(text.cbegin(), text.cend(), std::ostream_iterator<mintchar_t>(out));
            } // if
            out.close();
        } else {
#ifdef _VERBOSE_DEBUG
            std::cerr << "wfPrim: Error opening file " << fn << ": " << strerror(errno) << std::endl;
            interp.print();
#endif
            ret = strerror(errno);
        } // else
        interp.returnString(is_active, ret);
    } // operator()
}; // wfPrim

// #(pb)
// ---------
// Print contents of current buffer to stderr.
//
// Returns: null.
class pbPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList&) {
        const EmacsBuffer& buf = mint_buffers.getCurBuffer();
        std::cerr << "Buffer number: " << buf.getBufNumber() << std::endl;
        std::cerr << "===== CONTENTS =====" << std::endl;
        std::copy(buf.begin(), buf.end(), std::ostream_iterator<mintchar_t>(std::cerr));
        std::cerr << "=== END CONTENTS ===" << std::endl;
        interp.returnNull(is_active);
    } // operator()
}; // pbPrim

// #(bi,X,Y,A,B)
// -------------
// Buffer insert.  Insert into the current buffer the text from buffer "X"
// between point and mark "Y".
//
// Returns: "A" if insertion is successful, "B" otherwise.
class biPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto b = args.nextArg(argi).getIntValue();
        auto &mark_str = args.nextArg(argi).getValue();
        auto &arg_success = args.nextArg(argi).getValue();
        bool failed = false; // Assume we will succeed
        if (!mark_str.empty()) {
            failed = true; // Now we assume we will fail
            mintchar_t m = mark_str[0];
            mintcount_t cur_buf = mint_buffers.getCurBuffer().getBufNumber();
            if (mint_buffers.selectBuffer(b)) {
                MintString text;
                bool did_copy = mint_buffers.getCurBuffer().readToMark(m, &text);
                mint_buffers.selectBuffer(cur_buf);
                if (did_copy && mint_buffers.getCurBuffer().insertString(text)) {
                    failed = false; // We did succeed after all
                } // if
            } // if
        } // if
        if (failed) {
            auto &arg_failure = args.nextArg(argi).getValue();
            interp.returnString(is_active, arg_failure);
        } else {
            interp.returnString(is_active, arg_success);
        }
    } // operator()
}; // biPrim

// #(st,X)
// -------
// Syntax table. Sets the syntax table to the form given by "X".
// Syntax bits are as follows:
//     bit 0  0 = blank, 1 = non-blank (used for word matching)
//     bit 1  0 = not newline, 1 = newline
//
// Returns: null
class stPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList&) {
        // FIXME: this currently does nothing
        interp.returnNull(is_active);
    } // operator()
}; // stPrim

// #(lp,X,Y,A,B)
// -------------
// Look pattern.  Set search pattern to "X".  If "A" is not null, then "X"
// should be a regular expression (otherwise it's a string).  If "B" is not
// null, then case should be folded.
// The following regular expression characters are supported:
//       '*'         Zero or more
//       '[a-z]'     Character class
//       '[~a-z]'    Not character class
//       '.'         Any character
//       '^'         Beginning of line
//       '$'         End of line
// FIXME: need to implement the following
//       '\(' '\)'   Grouping (does not work with closures)
//       '\|'        Alternation
//       '\n'        New-line (does not have to appear at end of regex)
//       '\`'        Beginning of buffer
//       '\''        End of buffer
//       '\b'        Beginning or end of word
//       '\B'        Not beginning or end of word
//       '\<'        Beginning of word
//       '\>'        End of word
//       '\w'        Word character
//       '\W'        Not word character
//
// Returns: "Y" in active mode if an error occurs (eg invalid regex
// syntax), otherwise null.
class lpPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &string = args.nextArg(argi).getValue();
        auto &error_string = args.nextArg(argi).getValue();
        auto is_regex = !args.nextArg(argi).getValue().empty();
        auto fold_case = !args.nextArg(argi).getValue().empty();
        bool success;
        if (is_regex) {
            // Not a regular expression as such
            success = mint_buffers.setSearchString(string, fold_case);
        } else {
            success = mint_buffers.setSearchRegex(string, fold_case);
        }
        if (success) {
            interp.returnNull(is_active);
        } else {
            interp.returnString(true, error_string);
        }
    } // operator()
}; // lpPrim

// #(l?,A,B,C,D,X,Y)
// -----------------
// Look and test.  "A", "B", "C" and "D" are marks.  The search occurs
// between marks "A" and "B".  If the string (set by #(lp,...)) is found,
// mark "C" is set to the start of the matched string, and "D" to the end.
// "A" defaults to the beginning of file, "B" defaults to end of file, if
// "C" is null, defaults to mark 0 and "D" defaults to mark 1.
//
// Returns: "X" if pattern is found, "Y" otherwise.
class lkPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &mark1_str = args.nextArg(argi).getValue();
        auto &mark2_str = args.nextArg(argi).getValue();
        auto &mark3_str = args.nextArg(argi).getValue();
        auto &mark4_str = args.nextArg(argi).getValue();
        mintchar_t mark1 = mark1_str.empty() ? '[' : mark1_str[0];
        mintchar_t mark2 = mark2_str.empty() ? ']' : mark2_str[0];
        mintchar_t mark3 = mark3_str.empty() ?  0  : mark3_str[0];
        mintchar_t mark4 = mark4_str.empty() ?  1  : mark4_str[0];
        auto &success_string = args.nextArg(argi).getValue();
        if (mint_buffers.search(mark1, mark2, mark3, mark4)) {
            interp.returnString(is_active, success_string);
        } else {
            auto &failure_string = args.nextArg(argi).getValue();
            interp.returnString(is_active, failure_string);
        } // else
    } // operator()
}; // lkPrim

// #(tr,X,Y)
// ---------
// Translate.  Translates from point to mark "X" using string "Y" as a
// translation character set.  Each character is read from the buffer, and
// if the ordinal value is less than the length of "Y", then it is replaced
// with this character.
//
// Returns: null
class trPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &mark1_str = args.nextArg(argi).getValue();
        auto &trstr = args.nextArg(argi).getValue();
        mintchar_t mark1 = mark1_str.empty() ? '[' : mark1_str[0];
        mint_buffers.getCurBuffer().translate(mark1, trstr);
        interp.returnNull(is_active);
    } // operator()
}; // trPrim

class asVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, interp.getIdleMax());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        interp.setIdleMax(getStringIntValue(val));
    } // setVal
}; // asVar

class clVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, mint_buffers.getCurBuffer().getPointLine() + 1);
    } // getVal
    void setVal(Mint&, const MintString& val) {
        int lineno = getStringIntValue(val);
        EmacsBuffer& cb = mint_buffers.getCurBuffer();
        cb.setPointLine(std::max(0, lineno - 1));
    } // setVal
}; // clVar

class csVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, mint_buffers.getCurBuffer().getColumn() + 1);
    } // getVal
    void setVal(Mint&, const MintString& val) {
        int colno = getStringIntValue(val);
        if (colno > 0) {
            mint_buffers.getCurBuffer().setColumn(colno - 1);
        } // if
    } // setVal
}; // csVar

class mbVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, ((mint_buffers.getCurBuffer().isModified() ? 1 : 0) |
                                     (mint_buffers.getCurBuffer().isWriteProtected() ? 2 : 0)));
    } // getVal
    void setVal(Mint&, const MintString& val) {
        int flags = getStringIntValue(val);
        mint_buffers.getCurBuffer().setModified((flags & 1) != 0);
        mint_buffers.getCurBuffer().setWriteProtected((flags & 2) != 0);
#ifdef _DEBUG
        if (flags & ~3) {
            std::cerr << "Unrecognised flags " << flags << " in mbVar.setVal" << std::endl;
        } // if
#endif
    } // setVal
}; // mbVar

class nlVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        const EmacsBuffer& cb = mint_buffers.getCurBuffer();
        return stringAppendNum(str, cb.countNewlines() + 1);
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set
    } // setVal
}; // nlVar

class pbVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        const EmacsBuffer& cb = mint_buffers.getCurBuffer();
        return stringAppendNum(str, (cb.getPointLine() + 1) * 100 / (cb.countNewlines() + 1));
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set
    } // setVal
}; // pbVar

class rsVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString s;
        return stringAppendNum(s, getCurBuffer().getPointRow());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getCurBuffer().setPointRow(getStringIntValue(val));
    } // setVal
}; // rsVar

class tcVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, mint_buffers.getCurBuffer().getTabWidth());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        mint_buffers.getCurBuffer().setTabWidth(getStringIntValue(val));
        // Value can't be set
    } // setVal
}; // pbVar


EmacsBuffer& getCurBuffer() {
    return mint_buffers.getCurBuffer();
} // getCurBuffer


void registerBufPrims(Mint& interp) {
    interp.addPrim("ba", std::make_shared<baPrim>());
    interp.addPrim("is", std::make_shared<isPrim>());
    interp.addPrim("pm", std::make_shared<pmPrim>());
    interp.addPrim("sm", std::make_shared<smPrim>());
    interp.addPrim("sp", std::make_shared<spPrim>());
    interp.addPrim("dm", std::make_shared<dmPrim>());
    interp.addPrim("rm", std::make_shared<rmPrim>());
    interp.addPrim("rc", std::make_shared<rcPrim>());
    interp.addPrim("mb", std::make_shared<mbPrim>());
    interp.addPrim("rf", std::make_shared<rfPrim>());
    interp.addPrim("wf", std::make_shared<wfPrim>());
    interp.addPrim("bi", std::make_shared<biPrim>());
    interp.addPrim("pb", std::make_shared<pbPrim>());
    interp.addPrim("st", std::make_shared<stPrim>());
    interp.addPrim("lp", std::make_shared<lpPrim>());
    interp.addPrim("l?", std::make_shared<lkPrim>());
    interp.addPrim("tr", std::make_shared<trPrim>());

    interp.addVar("cl", std::make_shared<clVar>());
    interp.addVar("cs", std::make_shared<csVar>());
    interp.addVar("mb", std::make_shared<mbVar>());
    interp.addVar("nl", std::make_shared<nlVar>());
    interp.addVar("pb", std::make_shared<pbVar>());
    interp.addVar("rs", std::make_shared<rsVar>());
    interp.addVar("tc", std::make_shared<tcVar>());
} // registerBufPrims

// EOF
