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

#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <functional>

#if defined(WIN32)
#include <windows.h>
#elif defined(__CYGWIN__)
#include <windows.h>
#else
#include <glob.h>
#include <sys/utsname.h>
#endif

using namespace std::placeholders;

#include "sysprim.h"

namespace {
    template <typename T>
    std::time_t as_time_t(T time) {
        using namespace std::chrono;
        auto sctp = time_point_cast<system_clock::duration>(time - T::clock::now() + system_clock::now());
        return system_clock::to_time_t(sctp);
    }

    template <typename T>
    std::string getTime(std::chrono::time_point<T> time) {
        std::time_t tt = as_time_t(time);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&tt), "%a %b %d %H:%M:%S %Y");
        return ss.str();
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
        std::error_code ec;
        auto abs_path = std::filesystem::absolute(path_name, ec);
        MintString ret;
        if (static_cast<bool>(ec)) {
            ret = path_name.c_str();
        } else {
            auto canon_path = std::filesystem::canonical(abs_path, ec);
            if (static_cast<bool>(ec)) {
                ret = abs_path.string().c_str();
            } else {
                ret = canon_path.string().c_str();
            }
        }
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
        interp.getIdleMax(); // Suppress warning for unused interp
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
        std::stringstream s;
        if (!fns.empty()) {
            std::string fileName { fns.cbegin(), fns.cend() };
            std::error_code ec1;
            auto status { std::filesystem::status(fileName, ec1) };
            std::error_code ec2;
            auto time { std::filesystem::last_write_time(fileName, ec2) };
            std::error_code ec3;
            auto size = std::filesystem::file_size(fileName, ec3);
            if (static_cast<bool>(ec3)) {
                size = 0;
            }
            if (!static_cast<bool>(ec1) && !static_cast<bool>(ec2)) {
                // Check our extra info flag
                bool extra_info = !args.nextArg(argi).empty();
                if (extra_info) {
                    bool is_dir = std::filesystem::is_directory(status);
                    bool is_file = std::filesystem::is_regular_file(status);
                    // Directory/system/read-only flags are all we can emulate here
                    // Bit 5: not used (archive attribute)
                    s << '0';
                    // Bit 4: directory flag
                    s << (is_dir ? '1' : '0');
                    // Bit 3: not used (volume label attribute)
                    s << '0';
                    // Bit 2: Make it look like a system file if it's not a regular file or dir
                    s << ((!is_dir && !is_file) ? '1' : '0');
                    // Bit 1: not used (hidden attribute)
                    s << '0';
                    // Bit 0: read only flag
                    // FIXME: implement this
                    s << '0';
                    s << getTime(time) << ' ' << size;
                } else {
                    s << getTime(time);
                }
            } // if
        } else {
            // Get current system time
            s << getTime(std::chrono::system_clock::now());
        } // else
        interp.returnString(is_active, MintString(s.str().c_str()));
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
        std::error_code ec;
        std::filesystem::rename(fn1, fn2, ec);
        if (static_cast<bool>(ec)) {
            std::string msg(ec.message());
            ret = msg.c_str();
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
        std::error_code ec;
        if (!std::filesystem::remove(f1.c_str(), ec)) {
            std::string msg(ec.message());
            ret = msg.c_str();
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
        if (_argv != 0 && _argc > 0) {
            interp.setFormValue(MintString("env.FULLPATH"), MintString(_argv[0]));
            MintString runline;
            std::for_each(_argv + 1, _argv + _argc, std::bind(appendArgv, &runline, _1));
            interp.setFormValue(MintString("env.RUNLINE"), runline);
        } // if
        if (_envp != 0) {
            for (int i = 0; _envp[i] != 0; ++i) {
                std::string env(_envp[i]);
                auto pos = env.find('=');
                if (pos != std::string::npos) {
                    MintString name("env.");

                    name.append(MintString(env.substr(0, pos).c_str()));
                    interp.setFormValue(name, MintString(env.substr(pos + 1).c_str()));
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

// cd
// --
// Set/get the current working directory.
class cdVar : public MintVar {
    MintString getVal(Mint&) const {
        auto cwd = std::filesystem::current_path();
        MintString ret(cwd.string().c_str());
        if (ret.size() > 1 && *(--ret.cend()) != '/') {
            ret.append('/');
        }
        return ret;
    } // getVal
    void setVal(Mint&, const MintString& val) {
        std::string dir(val.cbegin(), val.cend());
        std::error_code ec;
        std::filesystem::current_path(dir, ec);
    } // setVal
}; // cdVar

// cn
// --
// Get computer name/type.  This value cannot be set.
class cnVar : public MintVar {
    MintString getVal(Mint&) const {
        std::stringstream s;
#ifdef WIN32
        s << "Windows";
#else
        utsname un;
        if (uname(&un) == 0) {
            s << un.sysname << " " << un.release;
        } else {
            s << "Unknown";
        }
#endif
        return MintString(s.str().c_str());
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set
    } // setVal
}; // cnVar

// is
// --
// Get/set "inhibit snow" flag for IBM CGA.
// This isn't a thing in any sane world anymore.
class isVar : public MintVar {
    MintString getVal(Mint&) const {
        return MintString("0");
    } // getVal
    void setVal(Mint&, const MintString&) {
        // Value can't be set - no need to inhibit snow!
    } // setVal
}; // isVar

// bp
// --
// Set the default bell pitch. If < 0 use visible bell.
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
