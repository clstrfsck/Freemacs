#include <iostream>

#include "mintprim.h"

#include "bufprim.h"
#include "winprim.h"
#include "mthprim.h"
#include "libprim.h"
#include "frmprim.h"
#include "strprim.h"
#include "sysprim.h"
#include "varprim.h"

int main(int argc, char **argv, char **envp) {
    Mint interp(
        // NOTE: Tabs are very important in this string
        "#(rd)#(ow,(\n"
        "Freemacs, a programmable editor - Version )##(lv,vn)(\n"
        "Copyright (C) Martin Sandiford 2003\n"
        "MINT code copyright (C) Russell Nelson 1986-1998\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under the conditions of the GNU General Public License.\n"
        "Type F1 C-c to see the conditions.\n"
        "))"
        "#(ds,Farglist,(SELF,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9))"
        "#(ds,Fsearch-path,(#(SELF-do,##(fm,env.PATH,;,(##(gn,env.PATH,1000))))"
        "#(rs,env.PATH)))"
        "#(mp,Fsearch-path,#(Farglist))"
        "#(ds,Fsearch-path-do,(#(==,arg1,,,("
        "\t#(==,#(ff,arg1/emacs.ed,;),,("
        "\t\t#(SELF,##(fm,env.PATH,;,(##(gn,env.PATH,1000))))"
        "\t),(#(ds,env.EMACS,arg1/)))"
        "))))"
        "#(mp,Fsearch-path-do,#(Farglist))"
        "#(ev)"

        // Did they tell us where to look?  If so, make sure it's got backslashes and a
        // trailing slash.

        "#(n?,env.EMACS,("
        "\t#(mp,env.EMACS,,/)"
        "\t#(ds,env.EMACS,##(env.EMACS,/))"
        "\t#(gn,env.EMACS,#(--,#(nc,##(env.EMACS)),1))"
        "\t#(==,##(go,env.EMACS)#(rs,env.EMACS),/,,("
        "\t\t#(ds,env.EMACS,##(env.EMACS)/)"
        "\t))"
        "))"

        // Did they tell us where to look?  If not, look where we came from for emacs.ed.

        "#(n?,env.EMACS,,("
        "\t#(ds,temp,##(env.FULLPATH))"
        "\t#(mp,temp,,emacs)"
        "\t#(==,#(ff,##(temp,emacs.ed),;),,,("
        "\t\t#(ds,env.EMACS,##(temp))"
        "\t))"
        "))"

        // Did we find emacs.ed in the directory we came from?  If not, search the
        // path for emacs.ed.

        "#(n?,env.EMACS,,(#(Fsearch-path)))"

        // Did we find it on the path?  If not, look in /emacs

        //"#(n?,env.EMACS,,(#(==,#(ff,/emacs/emacs.ed,;),,,(#(ds,env.EMACS,/emacs/)))))"

        "#(an,Loading #(env.EMACS)emacs.ed...)"
        "#(==,#(ll,#(env.EMACS)emacs.ed),,("
        "\t#(an,Starting editor...)"
        "\t#(##(lib-name)&setup)"
        "),("
        "\t#(an)"
        "\t#(ow,(\n"
        "Cannot find the Freemacs .ED files))"
        "\t#(==,#(rf,#(env.EMACS)boot.min),,("
        "\t\t#(ow,(, but we did find the boot files.\n"
        "Compiling the .ED files from the .MIN sources...\n"
        "))"
        "\t\t#(sp,[)#(rm,])#(dm,])"
        "\t),("
        "\t\t#(ow,("
        ". - Set the environment string EMACS to the subdirectory\n"
        "containing the Freemacs .ED files.  For example, EMACS=/emacs/\n"
        "Press any key to exit...))"
        "\t\t#(it,10000)#(ow,(\n))#(hl,1)"
        "\t))"
        "))"
        );
    try {
        registerBufPrims(interp);
        registerWinPrims(interp);
        registerMthPrims(interp);
        registerLibPrims(interp);
        registerFrmPrims(interp);
        registerStrPrims(interp);
        registerSysPrims(interp, argc, argv, envp);
        registerVarPrims(interp);
        while (true) {
            interp.scan();
        } // while
    } catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        interp.print(std::cerr);
    } // catch
    return 0;
} // main

// EOF
