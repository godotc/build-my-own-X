#include <algorithm>
#include <asm-generic/ioctls.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <tuple>
#include <unistd.h>
#include <vector>

#include "math.h"
#include <termio.h>

#include "assert.h"

#include "base.h"
#include "mod.h"
#include "util.h"
#include "vim_mode.h"

#include <fstream>
#include <string.h>

namespace ced {

#define CTRL_KEY(x) ((x)&0x1f)

struct EditorContext {
    // cursor x/y pos
    int cx = 0, cy = 0;
    int rows = 24;
    int cols = 80;

    // recored the current first line num
    int row_offset = 0, column_offset = 0;

    EKeyBinding    keybinding = EKeyBinding::Vim;
    EVimEditorMode mode       = EVimEditorMode::Normal;

    String              filebuffer;
    std::vector<String> lines;

    bool is_display_linenumber = true;
};

struct DeferScreenCleaner {
    ~DeferScreenCleaner()
    {
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
    }
};

enum class EOperation
{
    CursorLeft,
    CursorRight,
    CursorUp,
    CursorDown,

    PageUp,
    PageDown,

    Delete,
};


template <class T>
int OperationToInt(T) = delete;
template <>
int OperationToInt(EOperation op)
{
    switch (op) {
    case EOperation::CursorLeft:
        return 'h';
    case EOperation::CursorRight:
        return 'l';
    case EOperation::CursorUp:
        return 'k';
    case EOperation::CursorDown:
        return 'j';
        break;

    case EOperation::PageUp:
    case EOperation::PageDown:
    case EOperation::Delete:
    default:
        break;
    }

    return '\x1nil';
}


struct Editor {
    String version = "0.0.1";

    RawMode       rawmode_keeper;
    EditorContext context;
    String        buffer;
    char          last_sequence[4];

    Editor()
    {
        // TODO: support normal keybind
        ASSERT(context.keybinding == EKeyBinding::Vim, "?");

        // int row, col;
        if (!get_window_size(context.rows, context.cols)) {
            die("get window size");
        }
        // printf("windows row & col: %d, %d\r\n", config.rows, config.cols);
        // sleep(3);

        //  preclean the screen
        write(STDOUT_FILENO, "\x1b[2J", 4);
    }

    ~Editor()
    {
        // just ensure the cursor to show after close
        // buffer.append("\x1b[?25l", 6);
    }

    void run()
    {
        while (1) {
            refresh_screen();
            bool ret = process_keypress();
            if (!ret) {
                write(STDOUT_FILENO, "\x1b[2J", 4);
                write(STDOUT_FILENO, "\x1b[H", 3);
                break;
            }
        }
    }

    void open(const char *filename)
    {
        if (!filename)
            return;

        std::fstream fs(filename);
        if (fs.fail()) {
            die("open file");
        }

        // how to read all lines into buffers?
        context.lines.clear();

        std::string str;
        while (std::getline(fs, str)) {
            // want to remove the "\r\n" at the end;
            int pos = str.rfind("\r\n");
            if (pos != std::string::npos) {
                str.resize(pos);
            }

            // context.lines.emplace_back(std::move(str));
            context.lines.push_back(str);

            // printf("%s\r\n", context.lines[0].c_str());
            // sleep(2);
        }

        fs.close();
    }

    void refresh_screen()
    {
        scroll();

        // hide the cursor on refresh
        // 'h' and 'l' "Set Mode, Reset Mode", used to turn on and turn off terminal
        // featrues
        // ?25: hide/dispay cursor
        buffer.append("\x1b[?25l", 6);
        {
            // \x1b: escape code or 27 in decimal => "\"
            // \[2J clean entire screen
            // \[1J clean to where the cursor is
            // \[0J clean from the cursor to the end of screen
            // 0 is default argument for J
            // write(STDOUT_FILENO, "\x1b[2J", 4);
            // buffer.append("\x1b[2J", 4);
            //
            //  We will clean the lines by conditions in the draw_rows(), with "\[*K"

            // Reset the cursor to top-left instead the bottom by the last line we
            // write() H (Cursor Position:
            // https://vt100.net/docs/vt100-ug/chapter3.html#CUP) <esc>[<col>;<row>H
            // defualt col;row => 1;1  AND start from 1 not 0
            // write(STDOUT_FILENO, "\x1b[H", 3);
            buffer.append("\x1b[H", 3);

            draw_rows();

            // to top-left
            // write(STDOUT_FILENO, "\x1b[H", 3);
            // buffer.append("\x1b[H", 3);

            // draw cursor in cx & cy, (move cursor to the x,y)
            char buf[32];
            int  len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH",
                                context.cy - context.row_offset + 1, context.cx + 1);
            assert(len > 0 && len < 32);
            buffer.append(buf, len);
        }
        buffer.append("\x1b[?25h", 6);

        // buffer to screen
        write(STDOUT_FILENO, buffer.data(), buffer.size());
        buffer.clear();

        // printf("col,row =  %d,%d\n", config.cols, config.rows);
    }

    void draw_rows()
    {
        // I DONT know why the 0 line cannot be drew on screen
        for (int y = 1; y < context.rows; ++y) {
            int file_line = y - 1 + context.row_offset;
            // draw the string in files
            if (file_line < context.lines.size()) {
                const auto line = context.lines[file_line];
                int        len  = line.size() - context.column_offset;

                if (context.is_display_linenumber) {
                    String linenum = std::to_string(file_line) + std::string("| ");

                    // HERE! FIX IT, i'm going to talk with grilfriend
                    len = std::max(0, (int)(len - linenum.size()));

                    buffer.append(linenum);
                }

                // len = std::min(len, context.cols);
                if (len > context.cols) {
                    len = context.cols;
                }

                buffer.append(line.data(), len);

                // printf(" %s\r\n", context.lines[0].c_str());
                // printf("----------------------len= %d, %s\r\n", len, buffer.c_str());
                // sleep(1);
            }

            else {
                // the flag
                if (y == context.rows / 3) {
                    char welcome[80];
                    int  len = snprintf(welcome, sizeof(welcome),
                                        "ced editor ---- version %s\r\n", version.data());
                    len      = std::min(len, context.cols);

                    int padding = (context.cols - len) / 2;
                    if (padding) {
                        buffer.push_back('~');
                        --padding;

                        while (padding--) {
                            buffer.push_back(' ');
                        }
                    }
                    buffer.append(welcome, len);
                }
                // status line
                else if (y == context.rows - 1) {
                    auto version = EnumToString(context.mode);
                    buffer.push_back('|');
                    buffer.append(version.c_str(), version.length());
                    buffer.push_back('|');

                    // bot-right escape words
                    // int padding = (context.cols - 2 * 2 + 7 - 24);
                    //  while (padding--) {
                    //    buffer.push_back(' ');
                    //  }
                    //  buffer.append(last_sequence, 3);
                }
                // normal empty line
                else {
                    buffer.append("~", 1);
                }
            }

            // Now we don't need to clear entire screen,
            // 'K': erase in line, and argurments:
            // / 2: whole line, 1: left to the cursor, 0: right to the cursor(default)
            buffer.append("\x1b[K", 4);

            // next line
            if (y < context.rows - 1) {
                buffer.append("\r\n", 2);
            }
        }

        // The last line no need "\r\n"
        // write(STDOUT_FILENO, "~", 1);
    }

    bool process_keypress()
    {
        int ch = handle_input();

        bool result = true;

        // globaly
        switch (ch) {
        case CTRL_KEY('q'):
            result = false;
            break;
        case CTRL_KEY('x'):
            die("FUCK");
            break;
        }

        if (is_cursor_controlled(context.mode)) {
            switch (EOperation(ch)) {
            case CursorLeft:
            case CursorRight:
            case CursorUp:
            case CursorDown:
                move_cursor(static_cast<EOperation>(ch));
                break;
            case PageUp:
            case PageDown:
            {
                int times = context.rows;
                while (times--) {
                    move_cursor(ch == EOperation::PageUp ? EOperation::CursorUp
                                                         : EOperation::CursorDown);
                }
                break;
            }
            case Delete:
                break;
            }
        }

        switch (context.mode) {
        case EVimEditorMode::Normal:
        {
            switch (ch) {
            case 'i':
            {
                context.mode = EVimEditorMode::Insert;
                break;
            }

            default:
                break;
            }
            break;
        }
        case EVimEditorMode::Visual:
        case EVimEditorMode::Select:
        case EVimEditorMode::Insert:
        {
            switch (ch) {
            case '\x1b':
                context.mode = EVimEditorMode::Normal;
                break;
            }
            break;
        }
        case EVimEditorMode::Commandline:
        case EVimEditorMode::Ex:
        case EVimEditorMode::Terminal:
            break;
        }

        return result;
    }

    template <class T>
    void move_cursor(T) = delete;
    void move_cursor(EOperation key)
    {
        switch (key) {
        case CursorLeft:
            if (context.cy != 0) {
                --context.cx;
            }
            break;
        case CursorRight:
            if (context.cy != context.rows - 1) {
                ++context.cx;
            }
            break;
        case CursorUp:
            if (context.cx != 0) {
                --context.cy;
            }
            break;
        case CursorDown:
            if (context.cx < context.lines.size()) {
                ++context.cy;
            }
            break;

        default:
            break;
        }
    }

    bool get_window_size(int &rows, int &cols)
    {
        // EASY way
        winsize ws;

        int ret = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
        if (ret != -1 || ws.ws_col != 0) {
            rows = ws.ws_row;
            cols = ws.ws_col;
            return true;
        }

        // Then try HARD way
        // move cursor to the bot-right corner
        // "cursor forwad 999, cursur down 998" C & B means stop on meeting
        // edges . or you  maybe the freaking 9999 inches size screen editor?
        if (12 != write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12)) {
            return false;
        }

        return get_curor_position(rows, cols);
    }

    bool get_curor_position(int &rows, int &cols)
    {
        char     buf[32];
        unsigned i = 0;

        // n Device status report
        int ret = write(STDOUT_FILENO, "\x1b[6n", 4);
        if (ret != 4) {
            return false;
        }

        // just tryna find the 'R' char
        while (i < sizeof(buf) - 1) {
            if (1 != read(STDIN_FILENO, &buf[i], 1)) {
                break;
            }
            if (buf[i] == 'R') {
                ++i;
                break;
            }
            ++i;
        }
        // make a c stirng
        buf[i] = '\0';

        // skip first char of \x1b (escape \)
        // It’s an escape character (27), followed by a [ character, and then
        // the actual response: 24;80R by your window size && current pos

        // we cant print the col and row by this
        // printf("\r\n&buf[1]: '%s' \r\n", &buf[1]);
        // sleep(3);

        if (buf[0] != '\x1b' || buf[1] != '[') {
            return false;
        }
        if (2 != sscanf(&buf[2], "%d;%d", &rows, &cols)) {
            return false;
        }

        return true;
    }

    int handle_input()
    {
        int  num_read;
        char ch;
        // loop read till not -1
        while (1) {
            num_read = read(STDIN_FILENO, &ch, 1);
            if (num_read != -1) {
                break;
            }
            // not EAGAIN continue lopping
            if (num_read == -1 && errno != EAGAIN) {
                die("read");
            }
        }
        // printf("%d\t", ch);

        memset(last_sequence, '\0', sizeof(last_sequence));
        // check escpae keys
        if ('\x1b' == ch) {
            if (1 != read(STDIN_FILENO, &last_sequence[0], 1))
                return '\x1b';
            if (1 != read(STDIN_FILENO, &last_sequence[1], 1))
                return '\x1b';

            if (last_sequence[0] == '[' && is_cursor_controlled(context.mode)) {
                if (last_sequence[1] >= '0' && last_sequence[1] <= 9) {
                    if (1 != read(STDIN_FILENO, &last_sequence[2], 1))
                        return '\x1b';

                    if (last_sequence[2] == '~') {
                        switch (last_sequence[1]) {
                        case '3':
                            return EOperation::Delete;
                        case '5':
                            return EOperation::PageUp;
                        case '6':
                            return EOperation::PageDown;
                        }
                    }
                }
                else {
                    switch (last_sequence[1]) {
                    // arrows  up,down,right,left -> A,B,C,D
                    case 'A':
                        return EOperation::CursorUp;
                    case 'B':
                        return EOperation::CursorDown;
                    case 'C':
                        return EOperation::CursorRight;
                    case 'D':
                        return EOperation::CursorLeft;
                    default:
                        break;
                    }
                }
            }
            return '\x1b';
        }

        switch (ch) {
        case CTRL_KEY('f'):
            return PageDown;
        case CTRL_KEY('b'):
            return PageUp;
        }

        // printf("%d\r\n", ch);
        // sleep(1);

        return ch;
    }

    void scroll()
    {
        // upward scrol
        if (context.cy < context.row_offset) {
            context.row_offset = context.cy;
        }
        // has been the next page, update offset
        if (context.cy >= context.row_offset + context.rows) {
            context.row_offset = context.cy - context.rows + 1;
        }
    }
};

} // namespace ced
