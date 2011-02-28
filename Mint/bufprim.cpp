#include <fstream>
#include <iterator>

#include <errno.h>
#include <string.h>

#include "bufprim.h"
#include "emacsbuffers.h"

namespace {
    EmacsBuffers mint_buffers;
}; // namespace

class baPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        // Second parameter is ignored, as buffers are stored packed
        int whattodo = args[1].getIntValue();
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

class isPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (mint_buffers.getCurBuffer().insertString(args[1].getValue())) {
            interp.returnString(is_active, args[2].getValue());
        } else {
            interp.returnNull(is_active);
        } // else
    } // operator()
}; // isPrim

class pmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        int whattodo = args[1].getIntValue();
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
            interp.returnString(true, args[2].getValue());
        } // else
    } // operator()
}; // pmPrim

class smPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& m1 = args[1].getValue();
        if (!m1.empty()) {
            const MintString& m2 = args[2].getValue();
            mint_buffers.getCurBuffer().setMark(m1[0], m2.empty() ? '.' : m2[0]);
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // smPrim

class spPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        mint_buffers.getCurBuffer().setPointToMarks(args[1].getValue());
        interp.returnNull(is_active);
    } // operator()
}; // spPrim

class dmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        mint_buffers.getCurBuffer().deleteToMarks(args[1].getValue());
        interp.returnNull(is_active);
    } // operator()
}; // dmPrim

class rmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString s;
        const MintString& m = args[1].getValue();
        if (!m.empty() && mint_buffers.getCurBuffer().readToMark(m[0], &s)) {
            interp.returnString(is_active, s);
        } else {
            interp.returnString(true, args[2].getValue());
        } // else
    } // operator()
}; // rmPrim

class rcPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& m = args[1].getValue();
        interp.returnInteger(is_active, !m.empty()
                                         ? mint_buffers.getCurBuffer().charsToMark(m[0]) : 0);
    } // operator()
}; // rcPrim

class mbPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& m = args[1].getValue();
        if (!m.empty() && mint_buffers.getCurBuffer().markBeforePoint(m[0])) {
            interp.returnString(is_active, args[2].getValue());
        } else {
            interp.returnString(is_active, args[3].getValue());
        } // else
    } // operator()
}; // mbPrim

class rfPrim : public MintPrim {
    static const mintcount_t BUF_SIZE = 4096;
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& s = args[1].getValue();
        std::string fn;
        std::copy(s.begin(), s.end(), std::back_inserter(fn));
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

class wfPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& s = args[1].getValue();
        std::string fn;
        std::copy(s.begin(), s.end(), std::back_inserter(fn));
#ifdef _VERBOSE_DEBUG
        std::cerr << "wfPrim: Writing file " << fn << std::endl;
#endif
        std::ofstream out(fn.c_str(), std::ios::binary);
        if (out.good()) {
            const MintString& m = args[2].getValue();
            mintchar_t mark = m.empty() ? '.' : m[0];
            MintString text;
            if (mint_buffers.getCurBuffer().readToMark(mark, &text)) {
                std::copy(text.begin(), text.end(), std::ostream_iterator<mintchar_t>(out));
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

class pbPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const EmacsBuffer& buf = mint_buffers.getCurBuffer();
        std::cerr << "Buffer number: " << buf.getBufNumber() << std::endl;
        std::cerr << "===== CONTENTS =====" << std::endl;
        std::copy(buf.begin(), buf.end(), std::ostream_iterator<mintchar_t>(std::cerr));
        std::cerr << "=== END CONTENTS ===" << std::endl;
        interp.returnNull(is_active);
    } // operator()
}; // pbPrim

class biPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& mark_str = args[2].getValue();
        mintcount_t return_arg = 3; // Assume we will succeed
        if (!mark_str.empty()) {
            return_arg = 4; // Now we assume we will fail
            mintchar_t m = mark_str[0];
            mintcount_t b = static_cast<mintcount_t>(args[1].getIntValue());
            mintcount_t cur_buf = mint_buffers.getCurBuffer().getBufNumber();
            if (mint_buffers.selectBuffer(b)) {
                MintString text;
                bool did_copy = mint_buffers.getCurBuffer().readToMark(m, &text);
                mint_buffers.selectBuffer(cur_buf);
                if (did_copy && mint_buffers.getCurBuffer().insertString(text)) {
                    return_arg = 3; // We did succeed after all
                } // if
            } // if
        } // if
        interp.returnString(is_active, args[return_arg].getValue());
    } // operator()
}; // biPrim

class stPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        // FIXME: this currently does nothing
        interp.returnNull(is_active);
    } // operator()
}; // stPrim

class lpPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (args[3].empty()) {
            // Not a regular expression as such
            mint_buffers.setSearchString(args[1].getValue(), !args[4].empty());
        } else {
            mint_buffers.setSearchRegex(args[1].getValue(), !args[4].empty());
        } // else
        interp.returnNull(is_active);
    } // operator()
}; // lpPrim

class lkPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        mintchar_t mark1 = args[1].empty() ? '[' : args[1].getValue()[0];
        mintchar_t mark2 = args[2].empty() ? ']' : args[2].getValue()[0];
        mintchar_t mark3 = args[3].empty() ? 0 : args[3].getValue()[0];
        mintchar_t mark4 = args[4].empty() ? 0 : args[4].getValue()[0];
        if (mint_buffers.search(mark1, mark2, mark3, mark4)) {
            interp.returnString(is_active, args[5].getValue());
        } else {
            interp.returnString(is_active, args[6].getValue());
        } // else
    } // operator()
}; // lkPrim

class trPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        mintchar_t mark1 = args[1].empty() ? '.' : args[1].getValue()[0];
        const MintString& trstr = args[2].getValue();
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
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, mint_buffers.getCurBuffer().getPointLine() + 1);
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        int lineno = getStringIntValue(val);
        EmacsBuffer& cb = mint_buffers.getCurBuffer();
        cb.setPointLine(std::max(0, lineno - 1));
    } // setVal
}; // clVar

class csVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, mint_buffers.getCurBuffer().getColumn() + 1);
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        int colno = getStringIntValue(val);
        if (colno > 0) {
            mint_buffers.getCurBuffer().setColumn(colno - 1);
        } // if
    } // setVal
}; // csVar

class mbVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, ((mint_buffers.getCurBuffer().isModified() ? 1 : 0) |
                                     (mint_buffers.getCurBuffer().isWriteProtected() ? 2 : 0)));
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
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
    MintString getVal(Mint& interp) const {
        MintString str;
        const EmacsBuffer& cb = mint_buffers.getCurBuffer();
        return stringAppendNum(str, cb.countNewlines() + 1);
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        // Value can't be set
    } // setVal
}; // nlVar

class pbVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        const EmacsBuffer& cb = mint_buffers.getCurBuffer();
        return stringAppendNum(str, (cb.getPointLine() + 1) * 100 / (cb.countNewlines() + 1));
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        // Value can't be set
    } // setVal
}; // pbVar

class rsVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString s;
        return stringAppendNum(s, getCurBuffer().getPointRow());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        getCurBuffer().setPointRow(getStringIntValue(val));
    } // setVal
}; // rsVar

class tcVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, mint_buffers.getCurBuffer().getTabWidth());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        mint_buffers.getCurBuffer().setTabWidth(getStringIntValue(val));
        // Value can't be set
    } // setVal
}; // pbVar


EmacsBuffer& getCurBuffer() {
    return mint_buffers.getCurBuffer();
} // getCurBuffer


void registerBufPrims(Mint& interp) {
    interp.addPrim("ba", new baPrim);
    interp.addPrim("is", new isPrim);
    interp.addPrim("pm", new pmPrim);
    interp.addPrim("sm", new smPrim);
    interp.addPrim("sp", new spPrim);
    interp.addPrim("dm", new dmPrim);
    interp.addPrim("rm", new rmPrim);
    interp.addPrim("rc", new rcPrim);
    interp.addPrim("mb", new mbPrim);
    interp.addPrim("rf", new rfPrim);
    interp.addPrim("wf", new wfPrim);
    interp.addPrim("bi", new biPrim);
    interp.addPrim("pb", new pbPrim);
    interp.addPrim("st", new stPrim);
    interp.addPrim("lp", new lpPrim);
    interp.addPrim("l?", new lkPrim);
    interp.addPrim("tr", new trPrim);

    interp.addVar("cl", new clVar);
    interp.addVar("cs", new csVar);
    interp.addVar("mb", new mbVar);
    interp.addVar("nl", new nlVar);
    interp.addVar("pb", new pbVar);
    interp.addVar("rs", new rsVar);
    interp.addVar("tc", new tcVar);
} // registerBufPrims

// EOF
