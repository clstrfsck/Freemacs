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

#ifndef _EMWINDOW_WIN32_H
#define _EMWINDOW_WIN32_H

#include "emacswindow.h"

#include <map>

class EmacsWindowWin32 : public EmacsWindow {
public:

    EmacsWindowWin32();
    virtual ~EmacsWindowWin32();

    // Announce and announceWin are used to write in the message area and
    // modeline respectively.
    void announce(const MintString& left, const MintString& right);
    void announceWin(const MintString& left, const MintString& right);

    // Overwrite overlays text on the screen
    void overwrite(const MintString& str);
    // gotoxy sets cursor position for overwrite
    void gotoxy(int x, int y);

    // Set/get foreground and background colours
    void setForeColour(int colour);
    int  getForeColour() const;
    void setBackColour(int colour);
    int  getBackColour() const;
    void setCtrlForeColour(int colour);
    int  getCtrlForeColour() const;

    // Columns and lines in current window
    mintcount_t getColumns() const;
    mintcount_t getLines() const;

    // Control trailing whitespace display
    void setWhitespaceDisplay(bool flag) { _show_wsp = flag;   }
    bool getWhitespaceDisplay()          { return _show_wsp;   }
    void setWhitespaceColour(int colour) { _wsp_fore = colour; }
    int  getWhitespaceColour()           { return _wsp_fore;   }

    // Redraw the window using the specified buffer.
    // Force redraw if 'force' is true
    void redisplay(EmacsBuffer& buf, bool force = false);

    // Check to see if input key is available, and read input key with timeout
    bool keyWaiting();
    MintString getInput(mintcount_t millisec);

    // Audible bell.  Unlikely that freq and millisec can be taken advantage of.
    void audibleBell(mintcount_t freq, mintcount_t millisec);

    // Visible bell.
    void visualBell(mintcount_t millisec);

    // Set/get bottom scroll percent.  When point moves into the bottom
    // number of lines expressed as a percentage, the window will be scrolled.
    mintcount_t getBotScrollPercent() const           { return _botScrollPercent; }
    void        setBotScrollPercent(mintcount_t perc) { _botScrollPercent = perc; }

    // Set/get bottom scroll percent.  When point moves into the top
    // number of lines expressed as a percentage, the window will be scrolled.
    mintcount_t getTopScrollPercent() const           { return _topScrollPercent; }
    void        setTopScrollPercent(mintcount_t perc) { _topScrollPercent = perc; }


private:
    HANDLE hStdOut;
    HANDLE hStdIn;

    // _overwriting set to true if we have overwrite output on the screen
    bool _overwriting;
    // Position for overwrite output
    int _ovy, _ovx;

    // Next colour pair to be allocated
    short _curr_colour_pair;

    // Normal foreground and background colours
    int _fore, _back;

    // Whitespace information
    int _wsp_fore;
    bool _show_wsp;

    // Control character foreground
    int _ctrl_fore;

    // Previous foreground and background colours for optimisation
    int _old_fore, _old_back;

    // Map of curses key codes to mint key names
    std::map<int, MintString> _decode_key;

    // Bottom and top margins expressed as percentage of lines on screen
    mintcount_t _botScrollPercent;
    mintcount_t _topScrollPercent;

    // Write line with tab expansion and control colours
    void writeLine(const EmacsBuffer& buf, mintcount_t bol, mintcount_t eol);

    // Helper function to set up colsole colours
    void setConsoleAttributes(int fg, int bg);

}; // EmacsWindowWin32
    
#endif // _EMWINDOW_WIN32_H

// EOF
