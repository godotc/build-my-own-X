#include <algorithm>
#include <asm-generic/ioctls.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <tuple>
#include <unistd.h>

#include "math.h"
#include <termio.h>

#include "assert.h"

#include "base.h"
#include "mode.h"
#include <string.h>

namespace ced {

#define CTRL_KEY(x) ((x)&0x1f)

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  throw std::runtime_error(s);
}

struct DeferScreenCleaner {
  ~DeferScreenCleaner() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
  }
};

struct RawMode {
  termios origin_mode;

  RawMode() {
    int ret = tcgetattr(STDOUT_FILENO, &origin_mode);
    if (-1 == ret) {
      die("tcgetattr");
    }

    auto mode = origin_mode;
    mode.c_iflag &= ~(
        IXON     // Software flow control: Ctrl-S stops data from being
                 // transmitted to the terminal until you press Ctrl-Q.
        | ICRNL  // Fix C-m: terminal is helpfully translating any carriage
                 // returns (13, '\r') inputted by the user into newlines (10,
                 // '\n'). Let’s turn off this feature.
        | BRKINT // Also disble C-c some what "break condition"
        | INPCK  // "party checking": may not apply to modern terminal emulators
        | ISTRIP // causes the 8th bit of each input byte to be stripped,
                 // meaning it will set it to 0. Probably already turned off
    );
    mode.c_oflag &= ~(OPOST // post processing? \n to \r\n tranlsation
    );
    mode.c_lflag &=
        ~(ECHO     // echo mode
          | ICANON // read line-by-line -> byte-by-byte
          | ISIG   // singals: C-c, C-z
          | IEXTEN // On some systems, when you type Ctrl-V, the terminal
                   // waits for you to type another character and then sends
                   // that character literally.
        );

    mode.c_cflag |= (CS8 // A BITMASK, not a flag,
                         // which we set using the bitwise-OR (|) operator
                         // unlike all the flags we are turning off. It sets
                         // the character size (CS) to 8 bits per byte.
    );

    // "control chars"
    // The read timeout, or will block infinitely
    mode.c_cc[VMIN] = 0;
    mode.c_cc[VTIME] = 1;

    ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &mode);
    if (-1 == ret) {
      die("tcgetattr");
    }
  }
  ~RawMode() {
    printf("Turnoff raw mode..\n");
    int ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_mode);

    if (-1 == ret) {
      die("tcgetattr");
    }
  }
};

struct EditorConfig {
  int cx = 0,
      cy = 0; // cursor x/y pos
  int rows = 24;
  int cols = 80;
  EEditorMode mode = EEditorMode::Normal;
  char last_sequence[4];
};

struct Editor {
  String version = "0.0.1";

  RawMode rawmode_keeper;
  EditorConfig context;
  String buffer;

  Editor() {
    // int row, col;
    if (!get_window_size(context.rows, context.cols)) {
      die("get window size");
    }
    // printf("windows row & col: %d, %d\r\n", config.rows, config.cols);
    // sleep(3);

    //  preclean the screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
  }

  ~Editor() {
    // just ensure the cursor to show after close
    // buffer.append("\x1b[?25l", 6);
  }

  void run() {
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

  void refresh_screen() {
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

      // still top-left
      // write(STDOUT_FILENO, "\x1b[H", 3);
      // buffer.append("\x1b[H", 3);
      // draw cursor in cx & cy, (move cursor to the x,y)
      char buf[32];
      int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", context.cx + 1,
                         context.cy + 1);
      assert(len > 0 && len < 32);
      buffer.append(buf, len);
    }
    buffer.append("\x1b[?25h", 6);

    // buffer to screen
    write(STDOUT_FILENO, buffer.data(), buffer.size());
    buffer.clear();

    // printf("col,row =  %d,%d\n", config.cols, config.rows);
  }

  void draw_rows() {
    for (int y = 0; y < context.rows; ++y) {

      // the flag
      if (y == context.rows / 3) {
        char welcome[80];
        int len = snprintf(welcome, sizeof(welcome),
                           "ced editor ---- version %s\r\n", version.data());
        len = std::min(len, context.cols);

        int padding = (context.cols - len) / 2;
        if (padding) {
          buffer.push_back('~');
          --padding;

          while (padding--) {
            buffer.push_back(' ');
          }
        }
        buffer.append(welcome, len);
      } else if (y == context.rows - 1) {
        auto version = EnumToString(context.mode);
        buffer.push_back('|');
        buffer.append(version.c_str(), version.length());
        buffer.push_back('|');

        // bot-right escape words
        // int padding = (context.cols - 2 * 2 + 7 - 24);
        //  while (padding--) {
        //    buffer.push_back(' ');
        //  }
        //  buffer.append(context.last_sequence, 3);
      } else {
        buffer.append("~", 1);
      }

      // Now we don't need to clear entire screen,
      // 'K': erase in line, and argurments:
      // / 2: whole line, 1: left to the cursor, 0: right to the cursor
      buffer.append("\x1b[K", 4);

      // next line
      if (y < context.rows - 1) {
        buffer.append("\r\n", 2);
      }
    }
    // The last line no need "\r\n"
    // write(STDOUT_FILENO, "~", 1);
  }

  bool process_keypress() {
    char ch = read_key();

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
      switch (ch) {
      case 'h':
      case 'l':
      case 'j':
      case 'k': {
        move_cursor(ch);
        break;
      }
      }
    }

    switch (context.mode) {
    case EEditorMode::Normal: {
      switch (ch) {
      case 'i': {
        context.mode = EEditorMode::Insert;
        break;
      }

      default:
        break;
      }
      break;
    }
    case EEditorMode::Visual:
    case EEditorMode::Select:
    case EEditorMode::Insert: {
      switch (ch) {
      case '\x1b':
        context.mode = EEditorMode::Normal;
        break;
      }
      break;
    }
    case EEditorMode::Commandline:
    case EEditorMode::Ex:
    case EEditorMode::Terminal:
      break;
    }

    return result;
  }

  void move_cursor(char key) {
    switch (key) {
    case 'h':
      if (context.cy != 0) {
        --context.cy;
      }
      break;
    case 'l':
      if (context.cy != context.rows - 1) {
        ++context.cy;
      }
      break;

    case 'j':
      if (context.cx != context.cols - 1) {
        ++context.cx;
      }
      break;
    case 'k':
      if (context.cx != 0) {
        --context.cx;
      }
      break;
    }
  }

  bool get_window_size(int &rows, int &cols) {
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

  bool get_curor_position(int &rows, int &cols) {
    char buf[32];
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

  char read_key() {
    int cnt;
    char ch;
    while (-1 == (cnt = read(STDIN_FILENO, &ch, 1))) {
      if (cnt == -1 && errno != EAGAIN) {
        die("read");
      }
    }

    memset(context.last_sequence, '\0', sizeof(context.last_sequence));
    // check escpae keys
    if ('\x1b' == ch) {
      if (1 != read(STDIN_FILENO, &context.last_sequence[0], 1))
        return '\x1b';
      if (1 != read(STDIN_FILENO, &context.last_sequence[1], 1))
        return '\x1b';

      if (context.last_sequence[0] == '[' &&
          is_cursor_controlled(context.mode)) {
        switch (context.last_sequence[1]) {
        // arrows  up,down,right,left -> A,B,C,D
        case 'A':
          ch = 'k';
          break;
        case 'B':
          ch = 'j';
          break;
        case 'C':
          ch = 'l';
          break;
        case 'D':
          ch = 'h';
          break;
        }
      }
    }

    return ch;
  }
};

} // namespace ced

int main(int argc, char **argv) {
  try {
    ced::Editor ed;
    ed.run();
  } catch (std::exception &e) {
    printf("%s\n", e.what());
    // print the erron string?
  }

  return 0;
}
