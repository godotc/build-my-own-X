#pragma once

#include "base.h"
#include <cstdint>
#include <type_traits>
namespace ced {

enum class EEditorMode : uint8_t
{
    Normal = BIT(0),
    // In Normal mode you can enter all the normal editor
    //  commands.  If you start the editor you are in this
    //  mode.  This is also known as command mode.

    Visual = BIT(1),
    // This is like Normal mode, but the movement commands
    //	extend a highlighted area.  When a non-movement
    //	command is used, it is executed for the highlighted
    //	area.  See |Visual-mode|.
    //	If the 'showmode' option is on "-- VISUAL --" is shown
    //	at the bottom of the window.

    Select = BIT(2),
    // This looks most like the MS-Windows selection mode.
    //  	Typing a printable character deletes the selection
    //  	and starts Insert mode.  See |Select-mode|.
    //  	If the 'showmode' option is on "-- SELECT --" is shown
    //  	at the bottom of the window.

    Insert = BIT(3),
    // In Insert mode the text you type is inserted into the
    //  	buffer.  See |Insert-mode|.
    //  	If the 'showmode' option is on "-- INSERT --" is shown
    //  	at the bottom of the window.

    Commandline = BIT(4),
    // In Command-line mode (also called Cmdline mode) you
    // can enter one line of text at the bottom of the
    // window.  This is for the Ex commands, ":", the pattern
    // search commands, "?" and "/", and the filter command,
    //"!".  |Cmdline-mode|

    Ex = BIT(5),
    // Like Command-line mode, but after entering a command
    //  you remain in Ex mode.  Very limited editing of the
    //  command line.  |Ex-mode|

    Terminal = BIT(6),
    // In Terminal mode all input (except CTRL-\) is sent to
    // the process running in the current |terminal| buffer.
    // If CTRL-\ is pressed, the next key is sent unless it
    // is CTRL-N (|CTRL-\_CTRL-N|) or CTRL-O (|t_CTRL-\_CTRL-O|).
    // If the 'showmode' option is on "-- TERMINAL --" is shown
    // at the bottom of the window.

    /*
      There are six ADDITIONAL modes.  These are variants of the
       BASIC modes:

                                                    *Operator-pending*
       *Operator-pending-mode*
                    Operator-pending mode	This is like Normal mode, but after an
       operator command has started, and Vim is waiting for a {motion} to specify
       the text that the operator will work on.

                    Replace mode		Replace mode is a special case of Insert
       mode.  You can do the same things as in Insert mode, but for each character
       you enter, one character of the existing text is deleted.  See
       |Replace-mode|. If the 'showmode' option is on
                    "-- REPLACE --" is shown at the bottom of the window.

                    Virtual Replace mode	Virtual Replace mode is similar to
       Replace mode, but instead of file characters you are replacing screen real
       estate.  See |Virtual-Replace-mode|. If the 'showmode' option is on "--
       VREPLACE --" is shown at the bottom of the window.

                    Insert Normal mode	Entered when CTRL-O is typed in Insert
       mode (see |i_CTRL-O|).  This is like Normal mode, but after executing one
       command Vim returns to Insert mode. If the 'showmode' option is on "--
       (insert) --" is shown at the bottom of the window.

                    Insert Visual mode	Entered when starting a Visual selection
       from Insert mode, e.g., by using CTRL-O and then "v", "V" or CTRL-V.  When
       the Visual selection ends, Vim returns to Insert mode. If the 'showmode'
       option is on "-- (insert) VISUAL --" is shown at the bottom of the window.

                    Insert Select mode	Entered when starting Select mode from
       Insert mode. E.g., by dragging the mouse or <S-Right>. When the Select mode
       ends, Vim returns to Insert mode. If the 'showmode' option is on "--
       (insert) SELECT --" is shown at the bottom of the window.

          */

};

inline bool is_cursor_controlled(EEditorMode mode)
{
    constexpr auto flags = (uint8_t)EEditorMode::Insert |
                           (uint8_t)EEditorMode::Terminal |
                           (uint8_t)EEditorMode::Ex;
    return (uint8_t)mode & (~flags);
}

template <class T>
String        EnumToString(T) = delete;
inline String EnumToString(EEditorMode mode)
{
    switch (mode) {
    case EEditorMode::Normal:
        return "Normal";
    case EEditorMode::Visual:
        return "Visual";
    case EEditorMode::Select:
        return "Select";
    case EEditorMode::Insert:
        return "Insert";
    case EEditorMode::Commandline:
        return "Commandline";
    case EEditorMode::Ex:
        return "Ex";
    case EEditorMode::Terminal:
        return "Terminal";
        break;
    }
    return "Unknown Mode";
}



} // namespace ced
