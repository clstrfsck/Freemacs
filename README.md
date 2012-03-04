# Freemacs

[![Build Status](https://secure.travis-ci.org/msandiford/Freemacs.png?branch=master)](http://travis-ci.org/msandiford/Freemacs)

* [Wikipedia entry](http://en.wikipedia.org/wiki/Freemacs)

## Description

Freemacs is an Emacs-style editor, originally written in assembler and a [TRAC-like](http://en.wikipedia.org/wiki/TRAC_programming_language) language called MINT.  The original assembler version ran on MS-DOS based systems and was written by Russell Nelson.

Freemacs is interesting in as much as the assembler code portion is relatively small, and most of the editing functionality is actually implemented in the embedded macro language.

I have re-implemented the driver portion in C++.  It's no longer small, but it is almost complete, certainly complete enough to compile and run the original MINT source files.

## Installing

I've managed to compile this using Visual C++ 9.0 (aka MS Visual Studio 2008) on Windows, g++ 4.0.8 on MacOS X 10.5, and g++ 4.2.1 on MacOS X 10.6.

You can also try building using the `Makefile` on Linux.  It works on my machine and on travis-ci, but you may need to tweak the locations of things, especially curses.h.

Once you have an executable, you will need to compile the MINT files.  This is most easily accomplished by navigating into the "Editor" directory and executing the compiled Freemacs.

Maybe sometime I will implement a "proper" autoconf based build.

There is no real "install" process.

## Requirements

There is some guesswork involved here, as I've only really compiled it on three machines:

* ncurses (Windows folks can use the included PDCurses)
* boost regex (Included)

## License

Russell Nelson's original Freemacs code (Editor/*.min) files are copyright Russell Nelson and are GPL licensed.  It's not clearly stated in the documentation, but based on the dates and timing, I'm assuming the GPL V2.

PDCurses is largely public domain, but for exact licensing, please refer to the README accompanying the software.

boost regex is distributed under the boost license http://www.boost.org/LICENSE_1_0.txt

The remainder of the code (directories Emacs, Mint, MintTest) was written by me and is released under GPL V2 as below.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
