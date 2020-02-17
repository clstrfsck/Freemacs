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

#include "mint.h"

#include <iostream>

MintString Mint::_std_default_string_key("#(d,#(g))");
MintString Mint::_std_default_string_nokey("#(k)#(d,#(g))");
const MintString Mint::_default_active("dflta");
const MintString Mint::_default_neutral("dfltn");
const MintString Mint::_empty_string("");

bool keyWaiting();

// Constructors and initial set up
Mint::Mint()
    : _idle_max(0), _idle_count(0),
      _default_string_key(_std_default_string_key),
      _default_string_nokey(_std_default_string_nokey) {
    _activeString.push_front(_default_string_nokey);
} // Mint::Mint

Mint::Mint(const MintString& s)
    : _idle_max(0), _idle_count(0),
      _default_string_key(_std_default_string_key),
      _default_string_nokey(_std_default_string_nokey) {
    _activeString.push_front(s);
} // Mint::Mint

// Variables and primitives
void Mint::addVar(const MintString& name, std::shared_ptr<MintVar> func) {
    _vars[name] = func;
} // addVar

MintString Mint::getVar(const MintString& varName) {
    MintString val;
    MintVarMap::iterator i = _vars.find(varName);
    if (i != _vars.end()) {
        val = i->second->getVal(*this);
    }
#ifdef _EXEC_DEBUG
    else {
        std::cerr << "Can't find variable '" << varName << "' while reading" << std::endl;
    } // else
#endif
    return val;
} // getVar

void Mint::setVar(const MintString& varName, const MintString& val) {
    MintVarMap::iterator i = _vars.find(varName);
    if (i != _vars.end()) {
        i->second->setVal(*this, val);
    }
#ifdef _EXEC_DEBUG
    else {
        std::cerr << "Can't find variable '" << varName << "' while setting" << std::endl;
    } // else
#endif
} // setVar


void Mint::addPrim(const MintString& name, std::shared_ptr<MintPrim> func) {
    _prims[name] = func;
} // addPrim


void Mint::returnNull(bool is_active) {
    // This method only does something if we compile a debug version.
#ifdef _EXEC_DEBUG
    _err << "** Function (" << (is_active ? "A" : "N") << ") returned null string\n";
#endif
} // returnNull

    // Run-time return values
void Mint::returnString(bool is_active, const MintString& s) {
#ifdef _EXEC_DEBUG
    _err << "** Function (" << (is_active ? "A" : "N") << ") returned: " << s << "\n";
#endif
    if (is_active) {
        _activeString.push_front(s);
    } else {
        _neutralString.append(s);
    } // else
} // returnString

void Mint::returnInteger(bool is_active, int n, int b) {
    MintString s;
    returnString(is_active, stringAppendNum(s, n, b));
} // returnInteger

void Mint::returnInteger(bool is_active, mintcount_t n, int b) {
    MintString s;
    returnString(is_active, stringAppendNum(s, n, b));
} // returnInteger

void Mint::returnInteger(bool is_active, const MintString& prefix, int n, int b) {
    MintString pref(prefix);
    returnString(is_active, stringAppendNum(pref, n, b));
} // returnInteger

void Mint::returnNForm(bool is_active, const MintString& formName, int n, const MintString& nFound) {
    MintFormMap::iterator i = _forms.find(formName);
    if (i != _forms.end()) {
        if (i->second.atEnd())
            returnString(true, nFound);
        else
            returnString(is_active, i->second.getN(n));
    } else {
        returnNull(is_active);
    } // else
} // getNForm

void Mint::returnFormList(bool is_active, const MintString& sep, const MintString& prefix) {
    MintFormMap::const_iterator finish = _forms.end();
    MintString s;
    if (!prefix.empty()) {
        mintcount_t psize = prefix.size();
        bool need_sep = false;
        for (MintFormMap::const_iterator i = _forms.lower_bound(prefix); i != finish; ++i) {
            const MintString& formName = i->first;
            if (formName.size() < psize) {
                // Can't possibly be equal to prefix
                break;
            } // if
            MintString::const_iterator endp = formName.begin();
            std::advance(endp, psize);
            if (!std::equal(formName.begin(), endp, prefix.begin())) {
                break;
            } // if
            if (need_sep)
                s.append(sep);
            s.append(formName);
            need_sep = true;
        } // for
    } else {
        MintFormMap::const_iterator i = _forms.begin();
        if (i != finish) {
            s.append(i->first);
            for (++i; i != finish; ++i) {
                s.append(sep);
                s.append(i->first);
            }
        } // for
    } // else
    returnString(is_active, s);
} // returnFormList


// Idle count and reload
void Mint::setIdleMax(int n) {
    if (n > 0) {
        _idle_max = _idle_count = n;
    } else {
        _idle_max = _idle_count = 0;
    } // else
} // setIdleCount

int Mint::getIdleMax() const {
    return _idle_max;
} // getIdleMax

int Mint::getIdleCount() const {
    return _idle_count;
} // getIdleCount

void Mint::idle() {
    if (_idle_count > 0) {
        _idle_count -= 1;
        if (_idle_count == 0) {
            _idle_count = _idle_max;
            _idle_string = MintString("#(Fauto-save)");
        } // if
    } // if
} // idle


// Run-time form manipulation
void Mint::setFormPos(const MintString& formName, mintcount_t n) {
    MintFormMap::iterator i = _forms.find(formName);
    if (i != _forms.end()) {
        i->second.setPos(n);
    } // if
} // resetForm

// FIXME: There is no good reason for this to be a part of the interpreter
void Mint::returnSegString(bool is_active, const MintString& ss, const MintArgList& args) {
    if (is_active) {
        // Do this in reverse, so that it ends up in the
        // right order on the front of the active string.
        for (MintString::const_reverse_iterator j = ss.rbegin(); j != ss.rend(); ++j) {
            // FIXME: Stupid magic numbers and hardcoded types
            umintchar_t ch = static_cast<umintchar_t>(*j);
            if (ch >= 0x80) {
                _activeString.push_front(args[ch - 0x80].getValue());
            } else {
                _activeString.push_front(ch);
            } // else
        } // for
    } else {
        for (MintString::const_iterator j = ss.begin(); j != ss.end(); ++j) {
            // FIXME: Stupid magic numbers and hardcoded types
            umintchar_t ch = static_cast<umintchar_t>(*j);
            if (ch >= 0x80) {
                _neutralString.append(args[ch - 0x80].getValue());
            } else {
                _neutralString.append(1, ch);
            } // else
        } // for
    } // else
} // returnSegString

// FIXME: This is a bit crappy.  Should probably return a bool perhaps?
const MintForm& Mint::getForm(const MintString& formName, bool* found) const {
    static const MintForm result;
    MintFormMap::const_iterator i = _forms.find(formName);
    if (i != _forms.end()) {
        if (found != 0)
            *found = true;
        return i->second;
    } // if
    if (found != 0) {
        *found = false;
    } // if
    return result;
} // getForm

// FIXME: This is a bit crappy.  Should probably return a bool perhaps?
MintForm& Mint::getForm(const MintString& formName, bool* found) {
    static MintForm result;
    MintFormMap::iterator i = _forms.find(formName);
    if (i != _forms.end()) {
        if (found != 0)
            *found = true;
        return i->second;
    } // if
    if (found != 0) {
        *found = false;
    } // if
    result = MintForm();
    return result;
} // getForm

void Mint::delForm(const MintString& formName) {
    MintFormMap::iterator i = _forms.find(formName);
    if (i != _forms.end()) {
        _forms.erase(i);
    } // if
} // delForm

void Mint::setFormValue(const MintString& formName, const MintString& value) {
    _forms[formName] = MintForm(value);
} // setFormValue


void Mint::print(std::ostream &out, bool include_all_forms) const {
    out << "Interpreter status" << std::endl;
    out << "==================" << std::endl;
    _activeString.print(out);
    _neutralString.print(out);
    out << std::endl;
    if (include_all_forms) {
        out << "Forms" << std::endl;
        out << "=====" << std::endl;
        for (MintFormMap::const_iterator i = _forms.begin(); i != _forms.end(); ++i) {
            out << i->first << std::endl;
            out << std::string(i->first.size(), '-') << std::endl;
            out << i->second << std::endl << std::endl;
        } // for
    } // if
} // print



bool Mint::copyToCloseParen(MintActiveString::iterator& start) {
    mintcount_t parens = 1;
    MintActiveString::iterator next = start;
    // Skip starting paren, we know it's there
    ++next;
    while (parens > 0) {
        if (next == _activeString.end()) {
            // Couldn't find it, reload
            return false;
        } // if
        mintchar_t ch = *next++;
        if (ch == '(')
            parens += 1;
        else if (ch == ')')
            parens -= 1;
    } // while
    // Remove opening and closing paren
#ifdef USE_MINTSTRING_ROPE
    std::copy(start + 1, next - 1, std::back_inserter(_neutralString));
#else
    _neutralString.append(start + 1, next - 1);
#endif
    start = next;
    return true;
} // Mint::copyToCloseParen


bool Mint::executeFunction() {
    /*
      9. If the character under the scan pointer is a right parenthesis,
	a function is ending.  Delete the right parenthesis, advance the
	scan pointer, and mark the rightmost character of the neutral
	string as the end of an argument and the end of a function.  Now
	the neutral string from the rightmost begin function marker to
	the just inserted end function marker constitutes a MINT
	function invocation.  (If there is no begin function marker in
	the neutral string, return to step 1 without giving an error.)
	The first argument is assumed to be the name of a MINT function.
	If the argument is two characters long, and is the name of a
	built-in function, that function is evaluated with the given
	arguments: extra arguments are ignored and missing ones are
	automatically supplied as the null string.  If the function is
	not built-in, a default built-in function is executed.  The
	result of the function is catenated to the right of the neutral
	string if the function was marked as neutral and to the left of
	the active string if marked active; in the latter case, the scan
	pointer is reset to the leftmost character of the new active
	string.  Return to step 2.
    */
    _neutralString.markEndFunction();
    MintArgList args;
    _neutralString.popArguments(args);
    MintArgList::const_iterator i = args.begin();
    MintArg::ArgType atype = i->getType();
    if (atype == MintArg::MA_NULL) {
        // If we get the sentinel, we abort and reload default form
        return false;
    } else {
#ifdef _EXEC_DEBUG
        {
            _err << "Execute function: " << i->getValue() << " with " << args.size()-1 << " arguments\n";
            int argn = 1;
            MintArgList::const_iterator j = i;
            for (++j; j != args.end(); ++j)
                _err << "  Arg " << argn++ << " (" << j->getType() << "): " << j->getValue() << "\n";
//            _activeString.print();
//            _neutralString.print();
        }
#endif
        bool is_active = (atype == MintArg::MA_ACTIVE);
        const MintString& value = i->getValue();
        MintPrimMap::const_iterator f = _prims.find(value);
        if (f != _prims.end()) {
            (*f->second)(*this, is_active, args);
        } else {
            MintFormMap::const_iterator f = _forms.find(value);
            if (f == _forms.end()) {
#ifdef _VERBOSE_DEBUG
                _err << "Can't find form '" << value << "' while executing" << std::endl;
#endif
                if (i->getType() == MintArg::MA_ACTIVE) {
                    f = _forms.find(_default_active);
                } else {
                    f = _forms.find(_default_neutral);
                } // if
            } // if
            if (f != _forms.end()) {
                // Only get the remaining portion
                const MintForm& form = f->second;
                MintString::const_iterator begp = form.begin();
                std::advance(begp, form.getPos());
                returnSegString(is_active, MintString(begp, form.end()), args);
            } // if
        } // else
    } // else
    return true;
} // Mint::executeFunction


void Mint::clear() {
    _idle_count = _idle_max;
    _idle_string.clear();
    _activeString.clear();
    _neutralString.clear();
} // clear


// Returns if active string is exhausted ->
// This should result in a reload of the default string
void Mint::scan() {
#ifdef _VERBOSE_DEBUG
    _err << "Reload active string with default" << std::endl;
#endif
    if (_activeString.empty()) {
        _neutralString.clear();
        if (!_idle_string.empty()) {
            _activeString.load(_idle_string);
            _idle_string.clear();
        } else {
            if (keyWaiting()) {
                _activeString.load(_default_string_key);
            } else {
                _activeString.load(_default_string_nokey);
            } // else
        } // else
    } // if
    MintActiveString::iterator here = _activeString.begin();
    while (here != _activeString.end()) {
        switch (*here) {

        case '\t':
        case '\r':
        case '\n':
            /*
              3. If the character under the scan pointer is a horizontal
              tab, carriage return, or line feed, delete it, advance the
              scan pointer, and return to step 2.
            */
            ++here;
            break;

        case '(':
            /*
              4. If the character under the scan pointer is a left
              parenthesis, delete it and scan forward until the matching
              right parenthesis is found. After all of the intervening
              characters have been moved without change to the neutral
              string, the right parenthesis deleted, and the scan pointer
              moved to the character following the right parenthesis,
              return to step 2.  If the matching right parenthesis cannot
              be found, go back to step 1 without giving an error.
            */
            if (!copyToCloseParen(here)) {
                return;
            } // if
            break;

        case ',':
            /*
              5. If the character under the scan pointer is a comma, delete
              it, mark the rightmost character of the neutral string as
              the end of one argument, advance the scan pointer, and
              return to step 2.
            */
            ++here;
            _neutralString.markArgument();
            break;

        case '#': {
            MintActiveString::iterator next = here;
            ++next;
            if ((next != _activeString.end()) && (*next == '(')) {
                /*
                  6. If the character under the scan pointer is a sharp sign
                  and the next character is a left parenthesis, an active
                  function is beginning.  Delete the sharp sign and the
                  left parenthesis, advance the scan pointer beyond them,
                  mark the rightmost character of the neutral string as
                  the beginning of both an argument and an active
                  function, and return to step 2.
                */
                here = ++next;
                _neutralString.markActiveFunction();
            } else if ((next != _activeString.end()) && (*next == '#') &&
                       ((next + 1) != _activeString.end()) && (*(next + 1) == '(')) {
                /*
                  7. If the character under the scan pointer is a sharp
                  sign and the next two characters are another sharp
                  sign and a left parenthesis, a neutral function is
                  beginning.  Delete the triple ##(, advance the scan
                  pointer beyond them, mark the rightmost character
                  of the neutral string as the beginning of both an
                  argument and a neutral function, and return to step
                  2.
                */
                here = next + 2;
                _neutralString.markNeutralFunction();
            } else {
                /*
                  8. If the character under the scan pointer is a sharp sign
                  that did not meet the conditions of step 6 or 7, move
                  it to the right end of the neutral string, advance the
                  scan pointer, and return to step 2.
                */
                ++here;
                _neutralString.append(1, '#');
            } // else
        } // case
            break;

        case ')':
            ++here;
            _activeString.erase(_activeString.begin(), here);
            if (!executeFunction()) {
                return;
            } // if
            here = _activeString.begin();
            break;

        default:
            /*
              10. If the character under the scan pointer did not meet any
              of the conditions of steps 3 through 9, attach it to the
              right of the neutral string, delete it from the active
              string, advance the scan pointer, and return to step 2.
            */
            _neutralString.append(1, *here);
            ++here;
            break;
        } // switch
    } // while
    _activeString.clear();
} // Mint::scan

// EOF
