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
#include <fstream>
#include <iterator>

#ifdef WIN32
# include <io.h>
#else
# include <unistd.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "libprim.h"

namespace {
    struct LibHdr {
        mintcount_t total_length;
        mintcount_t name_length;
        mintcount_t reserved;
        mintcount_t form_pos;
        mintcount_t data_length;
    }; // LibHdr
} // namespace

#ifdef O_BINARY
# define FILE_MODE O_BINARY
#else
# define FILE_MODE 0
#endif

class slPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& s = args[1].getValue();
        std::string fn(s.begin(), s.end());
#ifdef _VERBOSE_DEBUG
        std::cerr << "slPrim: Saving library file " << fn << std::endl;
#endif
        std::ofstream out(fn.c_str(), std::ios::binary);
        if (out.good()) {
            if (args.size() > 3) {
                MintArgList::const_iterator start = args.begin();
                std::advance(start, 2);
                MintArgList::const_iterator finish = args.begin();
                std::advance(finish, args.size() - 1);
                for (MintArgList::const_iterator i = start; i != finish; ++i) {
                    bool found;
                    const MintString& formName = i->getValue();
                    const MintForm& form = interp.getForm(formName, &found);
                    if (found) {
                        LibHdr hdr;
                        hdr.total_length = sizeof(hdr) + formName.size() + form.size();
                        hdr.name_length = formName.size();
                        hdr.reserved = 0;
                        hdr.form_pos = form.getPos();
                        hdr.data_length = form.size();
                        out.write(reinterpret_cast<const char *>(&hdr), sizeof(hdr));
                        std::copy(formName.begin(), formName.end(),
                                  std::ostream_iterator<mintchar_t>(out));
                        std::copy(form.begin(), form.end(),
                                  std::ostream_iterator<mintchar_t>(out));
                    } // if
                } // for
            } // if
            out.close();
        } else {
#ifdef _VERBOSE_DEBUG
            std::cerr << "slPrim: Error opening file " << fn << ": " << strerror(errno) << std::endl;
#endif
            ret = strerror(errno);
        } // else
        interp.returnString(is_active, ret);
    } // operator()
}; // slPrim

class llPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& s = args[1].getValue();
        std::string fn(s.begin(), s.end());
#ifdef _VERBOSE_DEBUG
        std::cerr << "llPrim: Loading library file " << fn << std::endl;
#endif
        int fd = open(fn.c_str(), O_RDONLY | FILE_MODE);
        if (fd >= 0) {
            std::vector<mintchar_t> buf;
            off_t file_off = lseek(fd, 0, SEEK_END);
            if (file_off != -1) {
                lseek(fd, 0, SEEK_SET);
                mintcount_t file_len = static_cast<mintcount_t>(file_off);
                buf.resize(file_len);
                if (read(fd, reinterpret_cast<char *>(&(buf[0])), file_off) == file_off) {
                    const mintchar_t *p = &(buf[0]);
                    while (file_len > sizeof(LibHdr)) {
                        const LibHdr *phdr = reinterpret_cast<const LibHdr *>(p);
                        p += sizeof(LibHdr) / sizeof(mintchar_t);
                        file_len -= sizeof(LibHdr);
                        mintcount_t name_size = phdr->name_length;
                        mintcount_t data_size = phdr->data_length;
                        if (file_len >= (name_size + data_size)) {
                            MintString formName(p, name_size);
                            MintString formValue(p + name_size, data_size);
                            p += (name_size + data_size) / sizeof(mintchar_t);
                            file_len -= name_size + data_size;
                            interp.setFormValue(formName, formValue);
                            interp.setFormPos(formName, phdr->form_pos);
                        } // if
                    } // while
                } // if
            } // if
            close(fd);
        } else {
#ifdef _VERBOSE_DEBUG
            std::cerr << "llPrim: Error opening file " << fn << ": " << strerror(errno) << std::endl;
            interp.print();
#endif
            ret = strerror(errno);
        } // else
        interp.returnString(is_active, ret);
    } // operator()
}; // llPrim


void registerLibPrims(Mint& interp) {
    interp.addPrim("ll", new llPrim);
    interp.addPrim("sl", new slPrim);
} // registerLibPrims

// EOF
