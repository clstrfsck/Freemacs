#ifndef _MINT_H
#define _MINT_H

#include "mintarg.h"
#include "mintform.h"
#include "mintprim.h"
#include "mintstring.h"

#ifdef UNORDERED_MAP
# ifdef NEED_TR1
#  ifdef NEED_TR1_DIR
#   include <tr1/unordered_map>
#  else
#   include <unordered_map>
#  endif
namespace std {
    using tr1::unordered_map;
};
# else
#  include <unordered_map>
# endif
#else
#  include <map>
#endif
#include <deque>
#include <iostream>
#include <algorithm>

class Mint
{
public:

    // Constructors and initial set up
    Mint();

    Mint(const MintString& s);

    virtual ~Mint();


    void addVar(const MintString& name, MintVar *func);
    void addPrim(const MintString& name, MintPrim *func);


    // Execution
    void scan();


    // Clear everything
    void clear();


    // Idle count and reload
    void setIdleMax(int n);
    int getIdleMax() const;
    int getIdleCount() const;
    void idle();


    // Variables
    MintString getVar(const MintString& varName);
    void setVar(const MintString& varName, const MintString& val);


    // Run-time return values
    void returnNull(bool /*is_active*/);
    void returnString(bool is_active, const MintString& s);
    void returnInteger(bool is_active, int n, int b = 10);
    void returnInteger(bool is_active, mintcount_t n, int b = 10);
    void returnInteger(bool is_active, const MintString& prefix, int n, int b = 10);
    void returnNForm(bool is_active, const MintString& formName, int n, const MintString& nFound);
    void returnFormList(bool is_active, const MintString& sep, const MintString& prefix);
    void returnSegString(bool is_active, const MintString& ss, const MintArgList& args);


    // Run-time form manipulation
    void setFormPos(const MintString& formName, mintcount_t n);
    const MintForm& getForm(const MintString& formName, bool* found = 0) const;
    MintForm& getForm(const MintString& formName, bool* found = 0);
    void delForm(const MintString& formName);
    void setFormValue(const MintString& formName, const MintString& value);

    void print(bool include_all_forms = false) const;


private:
#ifdef UNORDERED_MAP
    typedef std::unordered_map<MintString,MintVar*>  MintVarMap;
    typedef std::unordered_map<MintString,MintPrim*> MintPrimMap;
    typedef std::unordered_map<MintString,MintForm>  MintFormMap;
#else
    typedef std::map<MintString,MintVar*>  MintVarMap;
    typedef std::map<MintString,MintPrim*> MintPrimMap;
    typedef std::map<MintString,MintForm>  MintFormMap;
#endif

    class MintActiveString {
    private:
        typedef std::deque<mintchar_t> MintActiveString_internal;
    public:
        typedef MintActiveString_internal::iterator iterator;
        typedef MintActiveString_internal::const_iterator const_iterator;

        void push_front(const MintString& s) {
            std::copy(s.rbegin(), s.rend(), std::front_inserter(_str));
        } // pushFront

        void push_front(mintchar_t ch) { _str.push_front(ch); }

        bool empty() { return _str.empty(); }
        size_t size() { return _str.size(); }
        void clear() { _str.clear(); }

        iterator begin() { return _str.begin(); }
        const_iterator begin() const { return _str.begin(); }

        iterator end() { return _str.end(); }
        const_iterator end() const { return _str.end(); }

        iterator erase(iterator pos) { return _str.erase(pos); }
        iterator erase(iterator first, iterator last) { return _str.erase(first, last); }

        void load(const MintString& s) {
            _str.clear();
            _str.insert(_str.begin(), s.begin(), s.end());
        } // load

        void print() const {
            std::cerr << "Active string: ";
            for (MintActiveString_internal::const_iterator i = _str.begin();
                 i != _str.end(); ++i) {
                std::cerr << *i;
            } // for
            std::cerr << std::endl;
        } // print

    private:
        MintActiveString_internal _str;
    }; // MintActiveString


    class MintNeutralString {
    public:

        MintNeutralString() { clear(); }

        void clear() {
            _args.clear();
            // Save sentinel value
            _args.push_front(MintArg(MintArg::MA_NULL));
            saveFunc();
        } // clear


        // Back insertion sequence model
        typedef MintString::reference reference;
        typedef MintString::const_reference const_reference;
        void push_back(mintchar_t ch) { _args.begin()->push_back(ch); }
        

        void append(const MintString& s) { _args.begin()->append(s); }
        void append(const mintchar_t* s, mintcount_t l) { _args.begin()->append(s, l); }
        void append(mintcount_t n, mintchar_t ch) { _args.begin()->append(n, ch); }
#ifndef USE_MINTSTRING_ROPE
        template <class II>
        void append(II first, II last) { _args.begin()->append(first, last); }
#endif

        void markArgument()
        { _args.push_front(MintArg(MintArg::MA_ARG)); }

        void markActiveFunction()
        { _args.push_front(MintArg(MintArg::MA_ACTIVE));  saveFunc(); }

        void markNeutralFunction()
        { _args.push_front(MintArg(MintArg::MA_NEUTRAL)); saveFunc(); }

        void markEndFunction()
        { _args.push_front(MintArg(MintArg::MA_END)); }

        void popArguments(MintArgList& args) {
            args.clear();
            // front_inserter means elements end up in reverse order
            std::copy(_args.begin(), _lastFunc, std::front_inserter(args));
            if (_lastFunc == _args.end()) {
                // If we copied everything out, nothing more to find
                // This also resets _lastFunc
                clear();
            } else {
                _args.erase(_args.begin(), _lastFunc);
                // FIXME: This is kind of a bug - we really need a stack here
                _lastFunc = std::find_if(_args.begin(), _args.end(), std::mem_fun_ref(&MintArg::isTerm));
                ++_lastFunc;
            } // else
        } // MintNeutralString::popArguments

        void print() const {
            std::cerr << "Neutral string:" << std::endl;
            for (MintArgList::const_iterator i = _args.begin(); i != _args.end(); ++i) {
                std::cerr << " Type: " << i->getType();
                if (_lastFunc == i)
                    std::cerr << "*";
                std::cerr << " Value: " << i->getValue() << std::endl;
            } // for
            std::cerr << std::endl;
        } // print

    private:
        MintArgList _args;
        MintArgList::iterator _lastFunc;
        void saveFunc() { _lastFunc = _args.begin(); ++_lastFunc; }
    }; // MintNeutralString



private:
    bool executeFunction();
    bool copyToCloseParen(MintActiveString::iterator& start);

    int _idle_max;
    int _idle_count;

    MintString _idle_string;
    MintString _default_string_key;
    MintString _default_string_nokey;

    MintActiveString _activeString;
    MintNeutralString _neutralString;

    MintFormMap _forms;

    MintVarMap  _vars;
    MintPrimMap _prims;

    static const MintString _default_active;
    static const MintString _default_neutral;
    static const MintString _empty_string;

    static MintString _std_default_string_key;
    static MintString _std_default_string_nokey;
    static void delVar(MintVarMap::value_type p);
    static void delPrim(MintPrimMap::value_type p);
}; // Mint


#endif // _MINT_H

// EOF
