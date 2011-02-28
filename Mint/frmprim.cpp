#include "frmprim.h"

class dsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.setFormValue(args[1].getValue(), args[2].getValue());
        interp.returnNull(is_active);
    } // operator()
}; // dsPrim

class gsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString form;
        MintArgList newargs;
        if (args.size() > 2) {
            newargs = args;
            // erase "gs" arg
            newargs.erase(newargs.begin());
            // Save form name, and erase name
            MintArgList::iterator i = newargs.begin();
            form = i->getValue();
            newargs.erase(i);
        } else {
            newargs.push_front(MintArg(MintArg::MA_END));
        } // else
        interp.returnSegString(is_active, interp.getForm(form).get(), newargs);
    } // operator()
}; // gsPrim

class goPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.returnNForm(is_active, 
                           args[1].getValue(),
                           1,
                           args[2].getValue());
    } // operator()
}; // goPrim

class gnPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.returnNForm(is_active, 
                           args[1].getValue(),
                           args[2].getIntValue(),
                           args[3].getValue());
    } // operator()
}; // gnPrim

class rsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.setFormPos(args[1].getValue(), 0);
        interp.returnNull(is_active);
    } // operator()
}; // rsPrim

class fmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        bool found;
        const MintString& formName = args[1].getValue();
        const MintForm& frm = interp.getForm(formName, &found);
        if (found) {
            const MintString& arg2 = args[2].getValue();
            MintString::const_iterator s = frm.begin() + frm.getPos();
            MintString::const_iterator f = arg2.empty() ? frm.end() : std::search(s, frm.end(), arg2.begin(), arg2.end());
            if (f != frm.end()) {
                interp.setFormPos(formName, (f - frm.begin()) + arg2.size());
                interp.returnString(is_active, MintString(s, f));
            } else {
                interp.returnString(true, args[3].getValue());
            } // else
        } else {
            interp.returnNull(is_active);
        } // else
    } // operator()
}; // fmPrim

class nxPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        bool found;
        interp.getForm(args[1].getValue(), &found);
        if (found) {
            interp.returnString(is_active, args[2].getValue());
        } else {
            interp.returnString(is_active, args[3].getValue());
        } // if
    } // operator()
}; // nxPrim

class lsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.returnFormList(is_active, args[1].getValue(), args[2].getValue());
    } // operator()
}; // lsPrim

class esPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        // We ned args[1] and args[args.size()-1] as our bounds
        if (args.size() > 2) {
            MintArgList::const_iterator start = args.begin();
            MintArgList::const_iterator finish = args.begin();
            std::advance(start, 1);
            std::advance(finish, args.size() - 1);
            // std::for_each would be good here, but we run into some
            // bustage with standard libraries trying to create
            // references to references.
            for (MintArgList::const_iterator i = start; i != finish; ++i) {
                interp.delForm(i->getValue());
            } // for
        }
        interp.returnNull(is_active);
    } // operator()
}; // esPrim

class mpPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (args.size() > 3) {
            bool found;
            const MintString& formName = args[1].getValue();
            MintString fv(interp.getForm(formName, &found));
            if (found) {
                MintArgList::const_iterator first = args.begin();
                std::advance(first, 2);
#ifdef USE_ARGS_SLIST
                MintArgList::const_iterator last = args.begin();
                std::advance(last, args.size() - 1);
#else
                MintArgList::const_iterator last = args.end();
                --last;
#endif
                // FIXME: Dodgy magic numbers and hardcoded types
                umintchar_t insch = 0x80;
                for (MintArgList::const_iterator i = first; i != last; ++i, ++insch) {
                    const MintString& av = i->getValue();
                    if (!av.empty()) {
                        const char* avp = av.c_str();
                        for (MintString::size_type pos = fv.find(avp, 0);
                             pos != MintString::npos; pos = fv.find(avp, pos + 1)) {
#ifdef USE_MINTSTRING_ROPE
                            fv.replace(pos, av.size(), static_cast<char>(insch));
#else
                            fv.replace(pos, av.size(), 1, static_cast<char>(insch));
#endif
                        } // for
                    } // if
                } // for
                interp.setFormValue(formName, fv);
            } // if
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // mpPrim

class hkPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (args.size() > 2) {
            MintArgList::const_iterator first = args.begin();
            std::advance(first, 1);
#ifdef USE_ARGS_SLIST
            MintArgList::const_iterator last = args.begin();
            std::advance(last, args.size() - 1);
#else
            MintArgList::const_iterator last = args.end();
            --last;
#endif
            for (MintArgList::const_iterator i = first; i != last; ++i) {
                bool found;
                MintString formValue = interp.getForm(i->getValue(), &found);
                if (found) {
                    MintArgList newargs;
#ifdef USE_ARGS_SLIST
                    newargs.push_front(*i);
                    newargs.insert_after(newargs.begin(), ++i, args.end());
#else
                    std::copy(i, args.end(), std::back_inserter(newargs));
#endif
                    interp.returnSegString(is_active, formValue, newargs);
                    return;
                } // if
            } // for
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // hkPrim


void registerFrmPrims(Mint& interp) {
    interp.addPrim("ds", new dsPrim);
    interp.addPrim("gs", new gsPrim);
    interp.addPrim("go", new goPrim);
    interp.addPrim("gn", new gnPrim);
    interp.addPrim("rs", new rsPrim);
    interp.addPrim("fm", new fmPrim);
    interp.addPrim("n?", new nxPrim);
    interp.addPrim("ls", new lsPrim);
    interp.addPrim("es", new esPrim);
    interp.addPrim("mp", new mpPrim);
    interp.addPrim("hk", new hkPrim);
} // registerFrmPrims

// EOF
