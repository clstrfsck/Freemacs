#ifndef _EMACSWINDOW_H
#define _EMACSWINDOW_H

#include "emacsbuffer.h"

class EmacsWindow {
public:

    EmacsWindow() {}
    virtual ~EmacsWindow() {}

    // Announce and announceWin are used to write in the message area and
    // modeline respectively.
    virtual void announce(const MintString& left, const MintString& right) = 0;
    virtual void announceWin(const MintString& left, const MintString& right) = 0;

    // Overwrite overlays text on the screen
    virtual void overwrite(const MintString& str) = 0;
    // gotoxy sets cursor position for overwrite
    virtual void gotoxy(int x, int y) = 0;

    // Set/get foreground and background colours
    virtual void setForeColour(int colour) = 0;
    virtual int  getForeColour() const  = 0;
    virtual void setBackColour(int colour) = 0;
    virtual int  getBackColour() const = 0;
    virtual void setCtrlForeColour(int colour) = 0;
    virtual int  getCtrlForeColour() const = 0;

    // Columns and lines in current window
    virtual mintcount_t getColumns() const = 0;
    virtual mintcount_t getLines() const = 0;

    // Control trailing whitespace display
    virtual void setWhitespaceDisplay(bool flag) = 0;
    virtual bool getWhitespaceDisplay() = 0;
    virtual void setWhitespaceColour(int colour) = 0;
    virtual int  getWhitespaceColour() = 0;

    // Redraw the window using the specified buffer.
    // Force redraw if 'force' is true
    virtual void redisplay(EmacsBuffer& buf, bool force = false) = 0;

    // Check to see if input key is available, and read input key with timeout
    virtual bool keyWaiting() = 0;
    virtual MintString getInput(mintcount_t millisec) = 0;

    // Audible bell.  Unlikely that freq and millisec can be taken advantage of.
    virtual void audibleBell(mintcount_t freq, mintcount_t millisec) = 0;

    // Visible bell.
    virtual void visualBell(mintcount_t millisec) = 0;

    // Set/get bottom scroll percent.  When point moves into the bottom
    // number of lines expressed as a percentage, the window will be scrolled.
    virtual mintcount_t getBotScrollPercent() const = 0;
    virtual void        setBotScrollPercent(mintcount_t perc) = 0;

    // Set/get bottom scroll percent.  When point moves into the top
    // number of lines expressed as a percentage, the window will be scrolled.
    virtual mintcount_t getTopScrollPercent() const = 0;
    virtual void        setTopScrollPercent(mintcount_t perc) = 0;
}; // EmacsWindow
    
extern EmacsWindow *emacs_window;
inline EmacsWindow& getEmacsWindow() { return *emacs_window; }

#endif // _EMACSWINDOW_H

// EOF
