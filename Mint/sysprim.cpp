#include <vector>
#include <iostream>

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
        struct tm *tmPtr = localtime_r(&time, &tms);
#else
        struct tm *tmPtr = localtime(&time);
#endif
        char timeStr[256]; // Hope this is sensible
        if (!strftime(timeStr, sizeof(timeStr), "%c", tmPtr))
            throw std::bad_alloc();
        return MintString(timeStr);
    }
}

class abPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& arg1 = args[1].getValue();
        std::string path_name(arg1.begin(), arg1.end());
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
                ret.push_back('/');
            } // if
        } // else

        interp.returnString(is_active, ret);
    } // operator()
}; // abPrim

class hlPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        int exitval = args[1].getIntValue(10);
#ifdef _DEBUG
        interp.print();
#endif
        // Program terminates here
        exit(exitval);
        interp.returnNull(is_active);
    } // operator()
}; // hlPrim

class ctPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& arg1 = args[1].getValue();
        MintString s;
        if (!arg1.empty()) {
            struct stat st;
            const MintString& fns = args[1].getValue();
            std::string fileName;
            std::copy(fns.begin(), fns.end(), std::back_inserter(fileName));
            if (stat(fileName.c_str(), &st) == 0) {
                s = getTime(st.st_mtime);
                // Check our extra info flag
                if (!args[2].getValue().empty()) {
                    s.append(1, ' ');
                    stringAppendNum(s, static_cast<mintcount_t>(st.st_size));
                    // Directory/system/read-only flags are all we can emulate here
                    MintString fileFlags("000 000");
                    // Bit 5: not used
                    fileFlags.append(1, '0');
                    // Bit 4: directory flag
                    fileFlags.append(1, (st.st_mode & S_IFDIR) ? '1' : '0');
                    // Bit 3: not used
                    fileFlags.append(1, '0');
                    // Bit 2: Make it look like a system file if it's not a regular file or dir
                    fileFlags.append(1, !(st.st_mode & (S_IFDIR | S_IFREG)) ? '1' : '0');
                    // Bit 1: not used
                    fileFlags.append(1, '0');
                    // Bit 0: read only flag
                    //FIXME
                    //fileFlags.append(1, (access(fileName.c_str(), W_OK) != 0) ? '1' : '0');
                    fileFlags.append(1, '0');
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

class ffPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& sep = args[2].getValue();
        const MintString& fpatStr = args[1].getValue();
        std::string fn;
        std::copy(fpatStr.begin(), fpatStr.end(), std::back_inserter(fn));
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
        if (pos == std::string::npos)
            pos = 0;
        else
            pos += 1;
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

class rnPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& fns1 = args[1].getValue();
        const MintString& fns2 = args[2].getValue();
        std::string fn1;
        std::copy(fns1.begin(), fns1.end(), std::back_inserter(fn1));
        std::string fn2;
        std::copy(fns2.begin(), fns2.end(), std::back_inserter(fn2));
        if (0 != ::rename(fn1.c_str(), fn2.c_str())) {
            ret = strerror(errno);
        } // if
        interp.returnString(is_active, ret);
    } // operator()
}; // rnPrim

class dePrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString &fn1 = args[1].getValue();
        std::string f1;
        std::copy(fn1.begin(), fn1.end(), std::back_inserter(f1));
        if (0 != unlink(f1.c_str())) {
            ret = strerror(errno);
        } // if
        interp.returnString(is_active, ret);
    } // operator()
}; // dePrim

class evPrim : public MintPrim {
public:
    evPrim(int argc, const char * const *argv, const char * const *envp)
        : _argc(argc), _argv(argv), _envp(envp) {
        // Nothing
    }

private:
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.setFormValue(MintString("env.SWITCHAR"), MintString("-"));
        // Unfortunately, we don't have this information
        interp.setFormValue(MintString("env.SCREEN"), MintString(""));
        if (_argv != 0) {
            interp.setFormValue(MintString("env.FULLPATH"), MintString(_argv[0]));

            MintString runline;
            std::for_each(_argv + 1, _argv + _argc, std::bind(appendArgv, runline, _1));
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

    static void appendArgv(MintString &runline, const char *argv) {
        runline.append(argv);
        runline.append(" ");
    } // appendArgv

    const int _argc;
    const char * const *_argv;
    const char * const *_envp;
}; // evPrim


class sdVar : public MintVar {
    MintString getVal(Mint& interp) const {
        const char *tmp = getenv("EMACSTMP");
        if (tmp == 0)
            tmp = getenv("TMP");
        if (tmp == 0)
            tmp = getenv("TEMP");
        if (tmp == 0)
            tmp = ".";
        return MintString(tmp);
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        // Value can't be set - no swap directory in modern O/S
    } // setVal
}; // sdVar

class cdVar : public MintVar {
    MintString getVal(Mint& interp) const {
        size_t wdsz = 256;
        std::vector<char> wd(wdsz);
        while ((wdsz > 0) && (getcwd(&(wd[0]), wdsz) == 0))
        {
            wdsz *= 2;
            wd.resize(wdsz);
        }
        MintString ret(&(wd[0]), strlen(&(wd[0])));
        if (ret.size() > 1)
            ret.push_back('/');
        return ret;
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        chdir(val.c_str());
    } // setVal
}; // cdVar

class cnVar : public MintVar {
    MintString getVal(Mint& interp) const {
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
    void setVal(Mint& interp, const MintString& val) {
        // Value can't be set
    } // setVal
}; // cnVar

class isVar : public MintVar {
    MintString getVal(Mint& interp) const {
        return MintString("0");
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        // Value can't be set - no need to inhibit snow!
    } // setVal
}; // isVar

class bpVar : public MintVar {
    MintString getVal(Mint& interp) const {
        return MintString("440");
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        // Value can't be set (bell pitch)
    } // setVal
}; // bpVar


void registerSysPrims(Mint& interp, int argc, const char * const *argv, const char * const *envp) {
    interp.addPrim("ab", new abPrim);
    interp.addPrim("hl", new hlPrim);
    interp.addPrim("ct", new ctPrim);
    interp.addPrim("ff", new ffPrim);
    interp.addPrim("rn", new rnPrim);
    interp.addPrim("de", new dePrim);
    interp.addPrim("ev", new evPrim(argc, argv, envp));

    interp.addVar("bp", new bpVar);
    interp.addVar("cd", new cdVar);
    interp.addVar("cn", new cnVar);
    interp.addVar("is", new isVar);
    interp.addVar("sd", new sdVar);
} // registerSysPrims

// EOF
