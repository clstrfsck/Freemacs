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

#include "emwindow-curses.h"

#include <iostream>
#include <algorithm>
#include <functional>

#ifdef WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

#define HAVE_PROTO
#if defined(XCURSES)
# include <xcurses.h>
#elsif defined(CURSES)
# include <curses.h>
#elif defined(NCURSES)
# include <ncurses/curses.h>
#else
# include <curses.h>
#endif

#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

using namespace std::placeholders;

// FIXME: Please.
EmacsWindow *emacs_window = 0;

namespace {
    class CursesSetupTeardown {
    public:
        CursesSetupTeardown() {
            emacs_window = new EmacsWindowCurses();
        } // CursesSetupTeardown
        ~CursesSetupTeardown() {
            delete emacs_window;
        } // ~CursesSetupTeardown
    };

    CursesSetupTeardown teardown;
}

WINDOW *_w(void *x) {
    return reinterpret_cast<WINDOW *>(x);
} // _w

EmacsWindowCurses::EmacsWindowCurses()
    : _overwriting(false), _ovy(0), _ovx(0),
      _curr_colour_pair(0),
      _fore(15), _back(0),
      _wsp_fore(15), _show_wsp(false),
      _ctrl_fore(11),
      _old_fore(-1), _old_back(-1),
      _botScrollPercent(0), _topScrollPercent(0) {
    if (isatty(1)) {
        _win = initscr();
        _has_colours = has_colors() ? true : false;
        if (_has_colours) {
            start_color();
        } // if
        raw();
        noecho();
#if !defined(WIN32) && !defined(XCURSES)
        nonl();
#endif
        intrflush(_w(_win), FALSE);
        keypad(_w(_win), TRUE);
#ifndef XCURSES
        idlok(_w(_win), TRUE);
        idcok(_w(_win), TRUE);
#endif
        scrollok(_w(_win), TRUE);
        clearok(_w(_win), TRUE);
        leaveok(_w(_win), FALSE);
        wsetscrreg(_w(_win), 0, LINES - 3);
        setCursesAttributes(_fore, _back);
        werase(_w(_win));
    } else {
        _win = 0;
        _has_colours = false;
    } // else

    // Fill out the defaults
    _decode_key[0x00] = MintString("C-@");
    for (mintchar_t i = 1; i < 32; ++i) {
        MintString key("C-");
        key.append(1, static_cast<mintchar_t>(i+'a'-1));
        _decode_key[i] = key;
    } // for
    for (mintchar_t i = 32; i < 127; ++i) {
        _decode_key[i] = MintString(1, i);
    } // for
    // Now fill in the specials
    _decode_key['\b'] = MintString("Back Space");
    _decode_key['\t'] = MintString("Tab");
    _decode_key['\n'] = MintString("Return");
    _decode_key['\r'] = MintString("Return");
    _decode_key[0x1B] = MintString("Escape");
    // ASCII keys that are inconvenient to do in ASCII
    _decode_key[','] = MintString("Comma");
    _decode_key['('] = MintString("LPar");
    _decode_key[')'] = MintString("RPar");
    // NCURSES decodes
    _decode_key[KEY_DOWN     ] = MintString("Down Arrow");
    _decode_key[KEY_UP       ] = MintString("Up Arrow");
    _decode_key[KEY_LEFT     ] = MintString("Left Arrow");
    _decode_key[KEY_RIGHT    ] = MintString("Right Arrow");
    _decode_key[KEY_HOME     ] = MintString("Home");
    _decode_key[KEY_BACKSPACE] = MintString("Back Space");
    _decode_key[KEY_F(1)     ] = MintString("F1");
    _decode_key[KEY_F(2)     ] = MintString("F2");
    _decode_key[KEY_F(3)     ] = MintString("F3");
    _decode_key[KEY_F(4)     ] = MintString("F4");
    _decode_key[KEY_F(5)     ] = MintString("F5");
    _decode_key[KEY_F(6)     ] = MintString("F6");
    _decode_key[KEY_F(7)     ] = MintString("F7");
    _decode_key[KEY_F(8)     ] = MintString("F8");
    _decode_key[KEY_F(9)     ] = MintString("F9");
    _decode_key[KEY_F(10)    ] = MintString("F10");
    _decode_key[KEY_F(11)    ] = MintString("F11");
    _decode_key[KEY_F(12)    ] = MintString("F12");
    _decode_key[KEY_F(13)    ] = MintString("S-F1");
    _decode_key[KEY_F(14)    ] = MintString("S-F2");
    _decode_key[KEY_F(15)    ] = MintString("S-F3");
    _decode_key[KEY_F(16)    ] = MintString("S-F4");
    _decode_key[KEY_F(17)    ] = MintString("S-F5");
    _decode_key[KEY_F(18)    ] = MintString("S-F6");
    _decode_key[KEY_F(19)    ] = MintString("S-F7");
    _decode_key[KEY_F(20)    ] = MintString("S-F8");
    _decode_key[KEY_F(21)    ] = MintString("S-F9");
    _decode_key[KEY_F(22)    ] = MintString("S-F10");
    _decode_key[KEY_F(23)    ] = MintString("S-F11");
    _decode_key[KEY_F(24)    ] = MintString("S-F12");
    _decode_key[KEY_DC       ] = MintString("Del");
    _decode_key[KEY_IC       ] = MintString("Ins");
    _decode_key[KEY_NPAGE    ] = MintString("Pg Dn");
    _decode_key[KEY_PPAGE    ] = MintString("Pg Up");
    _decode_key[KEY_END      ] = MintString("End");
} // EmacsWindowCurses

EmacsWindowCurses::~EmacsWindowCurses() {
    if (_win) {
        endwin();
#ifdef XCURSES
        XCursesExit();
#endif
    } // if
} // ~EmacsWindowCurses

mintcount_t EmacsWindowCurses::getColumns() const {
    if (_win) {
        return COLS;
    } // if
    return 80;
} // EmacsWindowCurses::getColumns

mintcount_t EmacsWindowCurses::getLines() const {
    if (_win) {
        // FIXME: Needs to deal with the split window eventually
        return LINES - 3;
    } // if
    return 24;
} // EmacsWindowCurses::getLines

void EmacsWindowCurses::writeLine(const EmacsBuffer& buf, mintcount_t bol, mintcount_t eol) {
    EmacsBuffer::const_iterator bolp = buf.begin() + bol;
    EmacsBuffer::const_iterator eolp = buf.begin() + eol;
    // Find the last non-space character
    EmacsBuffer::const_iterator nwsp = eolp;
    while (bolp != nwsp) {
        mintchar_t ch = *--nwsp;
        if (ch != '\t' && ch != ' ') {
            break;
        } // if
    } // while
    mintcount_t cur_col = 0;
    mintcount_t leftcol = buf.getLeftColumn();
    while ((cur_col < leftcol) && (bolp != eolp)) {
        cur_col += buf.charWidth(cur_col, *bolp++);
    } // while
    while ((cur_col < leftcol+COLS) && (bolp != eolp)) {
        umintchar_t ch = *bolp++;
        if (ch == 0x09) {
            mintcount_t tabw = buf.charWidth(cur_col, ch);
            tabw = std::min(tabw, leftcol + COLS - cur_col);
            setCursesAttributes(_fore, _back);
            chtype ch = (_show_wsp && (bolp > nwsp)) ? ACS_BULLET : ' ';
            for (mintcount_t i = 0; i < tabw; ++i) {
                waddch(_w(_win), ch);
            } // for
            cur_col += tabw;
        } else if (ch < 0x20) {
            setCursesAttributes(_ctrl_fore, _back);
            waddch(_w(_win), (ch + '@'));
            ++cur_col;
        } else if (ch == 0x20) {
            setCursesAttributes(_fore, _back);
            waddch(_w(_win), (_show_wsp && (bolp > nwsp)) ? ACS_BULLET : ' ');
            ++cur_col;
        } else {
            setCursesAttributes(_fore, _back);
            waddch(_w(_win), ch);
            ++cur_col;
        } // else
    } // while
    if (cur_col < leftcol+COLS) {
        setCursesAttributes(_fore, _back);
        wclrtoeol(_w(_win));
    } // if
} // EmacsWindowCurses::writeLine

void EmacsWindowCurses::redisplay(EmacsBuffer& buf, bool force) {
    if (_win) {
        _overwriting = false;
        if (force) {
            touchwin(_w(_win));
        } // if
        buf.forcePointInWindow(LINES - 2, COLS, _topScrollPercent, _botScrollPercent);
        mintcount_t curline = buf.getMarkPosition(EmacsBuffer::MARK_TOPLINE);
        mintcount_t screen_line = buf.countNewlines(curline, buf.getMarkPosition(EmacsBuffer::MARK_POINT));
        mintcount_t screen_col = buf.getColumn() - buf.getLeftColumn();
        for (int i = 0; i < LINES - 2; ++i) {
            wmove(_w(_win), i, 0);
            mintcount_t eol = buf.getMarkPosition(EmacsBuffer::MARK_EOL, curline);
            writeLine(buf, curline, eol);
            curline = buf.getMarkPosition(EmacsBuffer::MARK_NEXT_CHAR, eol);
        } // for
        wmove(_w(_win), screen_line, screen_col);
        //FIXME: wrefresh(_w(_win));
    } // if
} // EmacsWindowCurses::redisplay

void EmacsWindowCurses::overwrite(const MintString& str) {
    if (_w(_win)) {
        if (!_overwriting) {
            _overwriting = true;
            _ovy = _ovx = 0;
        } // if
        setCursesAttributes(_fore, _back);
        wmove(_w(_win), _ovy, _ovx);
        // FIXME: Control codes and high chars are written incorrectly
        std::for_each(str.begin(), str.end(), std::bind(waddch, _w(_win), _1));
        getyx(_w(_win), _ovy, _ovx);
    } else {
        std::cout << str;
    } // else
} // EmacsWindowCurses::overwrite

void EmacsWindowCurses::gotoxy(int x, int y) {
    if (_w(_win)) {
        if (!_overwriting) {
            _overwriting = true;
        } // if
        _ovy = std::max(0, std::min(y, LINES - 1));
        _ovx = std::max(0, std::min(x, COLS - 1));
        wmove(_w(_win), _ovy, _ovx);
    } // if
} // EmacsWindowCurses::gotoxy

bool EmacsWindowCurses::keyWaiting() {
    if (_w(_win)) {
#ifndef XCURSES
        nodelay(_w(_win), true);
        wtimeout(_w(_win), 0);
        int ch = wgetch(_w(_win));
        if (ch != ERR) {
            ungetch(ch);
            return true;
        } // if
#endif
    } // if
    return false;
} // EmacsWindowCurses::keyWaiting

MintString EmacsWindowCurses::getInput(mintcount_t millisec) {
    if (_w(_win)) {
        if (millisec < 10) {
            nodelay(_w(_win), true);
            wtimeout(_w(_win), 0);
        } else {
            nodelay(_w(_win), false);
            wtimeout(_w(_win), millisec);
        } // else
        // FIXME: wrefresh(_w(_win));
        int ch = wgetch(_w(_win));
        if (ch == ERR) {
            return MintString("Timeout");
        } else {
            std::map<int,MintString>::const_iterator i = _decode_key.find(ch);
            if (i == _decode_key.end()) {
                return MintString("Unknown");
            } else {
                return i->second;
            } // else
        } // if
    } else {
        if (millisec > 0) {
            char ch;
            std::cin >> ch;
            return MintString(1, ch);
        } else {
            return MintString("Timeout");
        } // else
    } // else
} // EmacsWindowCurses::getInput

void EmacsWindowCurses::announce(const MintString& left, const MintString& right) {
    if (_w(_win)) {
        int n = std::min(left.size(), static_cast<size_t>(COLS - 1));
        setCursesAttributes(_fore, _back);
        wmove(_w(_win), LINES - 1, 0);
        std::for_each(left.begin(), left.begin() + n, std::bind(waddch, _w(_win), _1));
        int y;
        int x;
        getyx(_w(_win), y, x);
        int m = std::min(right.size(), static_cast<size_t>(COLS - (n + 1)));
        std::for_each(right.begin(), right.begin() + m, std::bind(waddch, _w(_win), _1));
        if ((n + m) < COLS) {
            wclrtoeol(_w(_win));
        } // if
        wmove(_w(_win), y, x);
        refresh();
    } else {
        std::cout << left << right << std::endl;
    } // else
} // EmacsWindowCurses::announce

void EmacsWindowCurses::announceWin(const MintString& left, const MintString& right) {
    if (_w(_win)) {
        int n = std::min(left.size(), static_cast<size_t>(COLS - 1));
        setCursesAttributes(_fore, _back);
        int y;
        int x;
        getyx(_w(_win), y, x);
        wmove(_w(_win), LINES - 2, 0);
        std::for_each(left.begin(), left.begin() + n, std::bind(waddch, _w(_win), _1));
        int m = std::min(right.size(), static_cast<size_t>(COLS - n));
        std::for_each(right.begin(), right.begin() + m, std::bind(waddch, _w(_win), _1));
        if ((n + m) < COLS) {
            wclrtoeol(_w(_win));
        } // if
        wmove(_w(_win), y, x);
        refresh();
    } // if
} // EmacsWindowCurses::announceWin

void EmacsWindowCurses::audibleBell(mintcount_t /*freq*/, mintcount_t /*millisec*/) {
    if (_w(_win)) {
        beep();
    } else {
        std::cout << '\007';
    } // else
} // EmacsWindowCurses::audibleBell

void EmacsWindowCurses::visualBell(mintcount_t /*millisec*/) {
    if (_w(_win)) {
        flash();
    } // if
} // EmacsWindowCurses::visualBell

namespace {
    const int colourXlat[8] = {
        COLOR_BLACK,  COLOR_BLUE,
        COLOR_GREEN,  COLOR_CYAN,
        COLOR_RED,    COLOR_MAGENTA,
        COLOR_YELLOW, COLOR_WHITE
    };
    inline int cursesColour(int colour) {
        return colourXlat[colour & 0x07];
    } // cursesColour
    inline int cursesBold(int colour) {
        return (colour & 0x08) ? A_BOLD : A_NORMAL;
    } // cursesColour
} // namespace

void EmacsWindowCurses::setCursesAttributes(int fo, int ba) {
    if (_has_colours && ((fo != _old_fore) || (ba != _old_back))) {
        _old_fore = fo;
        _old_back = ba;
        int forecolour = cursesColour(fo);
        int forebold = cursesBold(fo);
        int backcolour = cursesColour(ba);
        short use_pair = COLOR_PAIRS;
        for (short i = 0; i < COLOR_PAIRS; ++i) {
            short f;
            short b;
            if ((ERR != pair_content(i, &f, &b)) && (f == forecolour) && (b == backcolour)) {
                use_pair = i;
                break;
            } // if
        } // for
        if (use_pair >= COLOR_PAIRS) {
            if (++_curr_colour_pair >= COLOR_PAIRS) {
                _curr_colour_pair = 1;
            } // if
            use_pair = _curr_colour_pair;
            init_pair(use_pair, forecolour, backcolour);
        } // if
        wattrset(_w(_win), COLOR_PAIR(use_pair) | forebold);
        // FIXME: I know this is not the preferred way, but how else?
        wbkgdset(_w(_win), COLOR_PAIR(use_pair) | forebold | ' ');
    } // if
} // EmacsWindowCurses::setCursesAttributes

// EOF
