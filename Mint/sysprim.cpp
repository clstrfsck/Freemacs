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

#include <vector>
#include <iostream>
#include <functional>

#if defined(WIN32)
#include <direct.h>
#include <windows.h>
#elif defined(__CYGWIN__)
#include <stdio.h>
#include <windows.h>
#else
#include <glob.h>
#include <limits.h>
#include <unistd.h>
#include <sys/utsname.h>
#endif

#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

using namespace std::placeholders;

#include "sysprim.h"

namespace {
    // Maximum size we will try to allocate for a path
    const size_t MAX_PATH_MAX = 64 * 1024;

    MintString getTime(time_t time) {
#ifdef _POSIX_C_SOURCE
        struct tm tms;
        const struct tm *tmPtr = localtime_r(&time, &tms);
#else
        const struct tm *tmPtr = localtime(&time);
#endif
        char timeStr[256]; // Hope this is sensible
        if (!strftime(timeStr, sizeof(timeStr), "%c", tmPtr))
            throw std::bad_alloc();
        return MintString(timeStr);
    }
}

// #(ab,X)
// -------
// Convert path given by "X" to an absolute path.
//
// Returns: the absolute path for "X", or "X" if an error occurs.
class abPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &arg1 = args.nextArg(argi).getValue();
        std::string path_name(arg1.cbegin(), arg1.cend());
#if defined(PATH_MAX)
        size_t path_max = PATH_MAX;
#elif defined(_PC_PATH_MAX)
        size_t path_max = pathconf(path_name.c_str(), _PC_PATH_MAX);
        if (path_max <= 0)
            path_max = 4096;
#else
        size_t path_max = 4096;
#endif
        path_max = std::min(path_max, MAX_PATH_MAX);
        MintString ret;

        std::vector<char> resolved_name(path_max);
#if defined(WIN32) || defined(__CYGWIN__)
        DWORD result = GetFullPathNameA(path_name.c_str(), path_max, &resolved_name[0], 0);
        bool failed = (result == 0 || result > path_max);
#else
        bool failed = NULL == realpath(path_name.c_str(), &resolved_name[0]);
#endif
        if (failed) {
            // Give up.  We should probably return some kind of error.
            ret = arg1;
        } else {
            ret = &resolved_name[0];
            struct stat st;
            if ((ret.size() > 1) &&
                (0 == stat(&resolved_name[0], &st)) &&
                (st.st_mode & S_IFDIR)) {
                ret.append('/');
            } // if
        } // else

        interp.returnString(is_active, ret);
    } // operator()
}; // abPrim

// #(hl,X)
// -------
// Halt.  Exit to operating system with return code "X" interpreted as
// decimal number.
//
// Returns: does not return
class hlPrim : public MintPrim {
    void operator()(Mint &interp, bool, const MintArgList &args) {
        auto argi = args.cbegin();
        auto exitval = args.nextArg(argi).getIntValue(10);
#ifdef _DEBUG
        interp.print();
#endif
        // Program terminates here
        exit(exitval);
    } // operator()
}; // hlPrim

// #(ct,X,Y)
// ---------
// Current time.  If "X" is null, returns system date/time.  If "X" is not
// null, it is used as a filename.  If "X" is specified, then if "Y" is
// non-null, binary file attributes and file size are included in the
// output string.
//
// Returns: ("X" null) System date in format "Sun Aug 08 09:01:03 2003".
//
// Returns: ("X" not null, "Y" null) Date of file "X" in above format, or
// null if no such file.
//
// Returns: ("X" not null, "Y" not null) Date of file "X" in above format,
// with file attributes prepended as 6 binary digits, and file size
// appended in the format "010000Sun Aug 08 09:01:03 2003 104323".  The
// bits of the file attributes have the following meanings if set:
//     Bit 0 - File is read only
//     Bit 1 - File is hidden
//     Bit 2 - File is a system file
//     Bit 3 - File is a volume label
//     Bit 4 - File is a directory
//     Bit 5 - File is ready for archiving (modified since backup)
class ctPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &fns = args.nextArg(argi).getValue();
        MintString s;
        if (!fns.empty()) {
            struct stat st;
            std::string fileName(fns.cbegin(), fns.cend());
            if (stat(fileName.c_str(), &st) == 0) {
                s = getTime(st.st_mtime);
                // Check our extra info flag
                bool extra_info = !args.nextArg(argi).empty();
                if (extra_info) {
                    s.append(' ');
                    stringAppendNum(s, static_cast<mintcount_t>(st.st_size));
                    // Directory/system/read-only flags are all we can emulate here
                    MintString fileFlags("000 000");
                    // Bit 5: not used
                    fileFlags.append('0');
                    // Bit 4: directory flag
                    fileFlags.append((st.st_mode & S_IFDIR) ? '1' : '0');
                    // Bit 3: not used
                    fileFlags.append('0');
                    // Bit 2: Make it look like a system file if it's not a regular file or dir
                    fileFlags.append(!(st.st_mode & (S_IFDIR | S_IFREG)) ? '1' : '0');
                    // Bit 1: not used
                    fileFlags.append('0');
                    // Bit 0: read only flag
                    //FIXME
                    //fileFlags.append(1, (access(fileName.c_str(), W_OK) != 0) ? '1' : '0');
                    fileFlags.append('0');
                    fileFlags.append(s);
                    s = fileFlags;
                } // if
            } // if
        } else {
            // Get current system time
            time_t now;
            time(&now);
            s = getTime(now);
        } // else
        interp.returnString(is_active, s);
    } // operator()
}; // ctPrim

// #(ff,X,Y)
// ---------
// Find file.  "X" is a literal string which may contain globbing
// characters. "Y" is a separator string used in the return value.
//
// Returns: List of matching files, separated by literal string "Y".
class ffPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        auto argi = args.cbegin();
        auto &fpatStr = args.nextArg(argi).getValue();
        auto &sep = args.nextArg(argi).getValue();
        std::string fn(fpatStr.cbegin(), fpatStr.cend());
#ifdef WIN32
        ::WIN32_FIND_DATAA fd;
        ::HANDLE hFind = ::FindFirstFileA(fn.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                ret.append(MintString(fd.cFileName));
                ret.append(sep);
            } while (::FindNextFileA(hFind, &fd));
            ::FindClose(hFind);
        } // if
#else
        std::string::size_type pos = fn.rfind('/');
        if (pos == std::string::npos) {
            pos = 0;
        } else {
            pos += 1;
        }
        glob_t gt;
        if (0 == glob(fn.c_str(), GLOB_NOSORT | GLOB_MARK, 0, &gt)) {
            for (mintcount_t i = 0; i < static_cast<mintcount_t>(gt.gl_pathc); ++i) {
                ret.append(MintString(gt.gl_pathv[i] + pos));
                ret.append(sep);
            } // for
            globfree(&gt);
        } // if
#endif
        interp.returnString(is_active, ret);
    } // operator()
}; // ffPrim

// #(rn,X,Y)
// ---------
// Rename file.  Rename file given by literal string "X" to "Y".
//
// Returns: null if successful, error text otherwise.
class rnPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        auto argi = args.cbegin();
        auto &fns1 = args.nextArg(argi).getValue();
        auto &fns2 = args.nextArg(argi).getValue();
        std::string fn1(fns1.cbegin(), fns1.cend());
        std::string fn2(fns2.cbegin(), fns2.cend());
        if (0 != ::rename(fn1.c_str(), fn2.c_str())) {
            ret = strerror(errno);
        } // if
        interp.returnString(is_active, ret);
    } // operator()
}; // rnPrim

// #(de,X)
// -------
// Delete file.  Delete file given by literal string "X".
//
// Returns: null if successful, error text otherwise.
class dePrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        auto argi = args.cbegin();
        auto &fns1 = args.nextArg(argi).getValue();
        std::string f1(fns1.cbegin(), fns1.cend());
        if (0 != unlink(f1.c_str())) {
            ret = strerror(errno);
        } // if
        interp.returnString(is_active, ret);
    } // operator()
}; // dePrim

// #(ev)
// -----
// Read environment.  This reads the operating system environment, and
// defines forms of the name "env.PATH" for each variable found in the
// environment.  In addition, the following forms are defined:
//     env.RUNLINE         The complete command line
//     env.SWITCHAR        The switch character (eg '-')
//     env.FULLPATH        The full path to the executable
//     env.SCREEN          The original contents of the screen
//
// Returns: null
class evPrim : public MintPrim {
public:
    evPrim(int argc, const char * const *argv, const char * const *envp)
        : _argc(argc), _argv(argv), _envp(envp) {
        // Nothing
    }

private:
    void operator()(Mint& interp, bool is_active, const MintArgList&) {
        interp.setFormValue(MintString("env.SWITCHAR"), MintString("-"));
        // Unfortunately, we don't have this information
        interp.setFormValue(MintString("env.SCREEN"), MintString(""));
        if (_argv != 0) {
            interp.setFormValue(MintString("env.FULLPATH"), MintString(_argv[0]));
            MintString runline;
            std::for_each(_argv + 1, _argv + _argc, std::bind(appendArgv, &runline, _1));
            interp.setFormValue(MintString("env.RUNLINE"), runline);
        } // if
        if (_envp != 0) {
            for (int i = 0; _envp[i] != 0; ++i) {
                const char *p = _envp[i];
                const char *q = strchr(p, '=');
                if (q != NULL) {
                    MintString name("env.");
                    name.append(MintString(p, q - p));
                    interp.setFormValue(name, MintString(q + 1));
                } // if
            } // for
        } // if
        interp.returnNull(is_active);
    } // operator()

    static void appendArgv(MintString *runline, const char *argv) {
        runline->append(argv);
        runline->append(" ");
    } // appendArgv

    const int _argc;
    const char * const *_argv;
    const char * const *_envp;
}; // evPrim


class sdVar : public MintVar {
    MintString getVal(Mint&) const {
        const char *tmp = getenv("EMACSTMP");
        if (tmp == 0)
            tmp = getenv("TMP");
        if (tmp == 0)
            tmp = getenv("TEMP");
        if (tmp == 0)
            tmp = ".";
        return MintString(tmp);
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set - no swap directory in modern O/S
    } // setVal
}; // sdVar

class cdVar : public MintVar {
    MintString getVal(Mint&) const {
        size_t wdsz = 256;
        std::vector<char> wd(wdsz);
        while ((wdsz > 0) && (getcwd(&(wd[0]), wdsz) == 0))
        {
            wdsz *= 2;
            wd.resize(wdsz);
        }
        MintString ret(&(wd[0]), strlen(&(wd[0])));
        if (ret.size() > 1)
            ret.append('/');
        return ret;
    } // getVal
    void setVal(Mint&, const MintString& val) {
        std::string dir(val.cbegin(), val.cend());
        chdir(dir.c_str());
    } // setVal
}; // cdVar

class cnVar : public MintVar {
    MintString getVal(Mint&) const {
#ifdef WIN32
        const char *name = "Win32";
#else
        const char *name = "Unknown";
        utsname un;
        if (uname(&un) == 0)
            name = un.sysname;
#endif
        return MintString(name);
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set
    } // setVal
}; // cnVar

class isVar : public MintVar {
    MintString getVal(Mint&) const {
        return MintString("0");
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set - no need to inhibit snow!
    } // setVal
}; // isVar

class bpVar : public MintVar {
    MintString getVal(Mint&) const {
        return MintString("440");
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set (bell pitch)
    } // setVal
}; // bpVar


void registerSysPrims(Mint& interp, int argc, const char * const *argv, const char * const *envp) {
    interp.addPrim("ab", std::make_shared<abPrim>());
    interp.addPrim("hl", std::make_shared<hlPrim>());
    interp.addPrim("ct", std::make_shared<ctPrim>());
    interp.addPrim("ff", std::make_shared<ffPrim>());
    interp.addPrim("rn", std::make_shared<rnPrim>());
    interp.addPrim("de", std::make_shared<dePrim>());
    interp.addPrim("ev", std::make_shared<evPrim>(argc, argv, envp));

    interp.addVar("bp", std::make_shared<bpVar>());
    interp.addVar("cd", std::make_shared<cdVar>());
    interp.addVar("cn", std::make_shared<cnVar>());
    interp.addVar("is", std::make_shared<isVar>());
    interp.addVar("sd", std::make_shared<sdVar>());
} // registerSysPrims

// EOF
