#include <asm-generic/ioctls.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <termios.h>
#include <tuple>
#include <unistd.h>

#include "math.h"
#include <termio.h>

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
  int rows = 24;
  int cols = 80;
};

struct Editor {
  RawMode rawmode_keeper;
  EditorConfig config;

  Editor() {
    // int row, col;
    if (!get_window_size(config.rows, config.cols)) {
      die("get window size");
    }
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
    // \x1b: escape code or 27 in decimal => "\"
    // \[2J clean entire screen
    // \[1J clean to where the cursor is
    // \[0J clean from the cursor to the end of screen
    // 0 is default argument for J
    write(STDOUT_FILENO, "\x1b[2J", 4);

    // Reset the cursor to top-left instead the bottom by the last line we
    // write() H (Cursor Position:
    // https://vt100.net/docs/vt100-ug/chapter3.html#CUP) <esc>[<col>;<row>H
    // defualt col;row => 1;1  AND start from 1 not 0
    write(STDOUT_FILENO, "\x1b[H", 3);

    draw_rows();

    // still top-left
    write(STDOUT_FILENO, "\x1b[H", 3);

    // printf("col,row =  %d,%d\n", config.cols, config.rows);
  }

  bool process_keypress() {
    char c = read_key();
    printf("%c", c);

    bool result = true;

    switch (c) {
    case CTRL_KEY('q'):
      result = false;
      break;
    case CTRL_KEY('x'):
      die("FUCK");
      break;
    default:
      break;
    }

    return result;
  }

  void draw_rows() {
    for (int y = 0; y < config.rows; ++y) {
      write(STDOUT_FILENO, "~\r\n", 3);
    }
  }

  bool get_window_size(int &rows, int &cols) {
#if 0 // easy way
    winsize ws;
    // WINSZ
    int ret = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (ret == -1 || ws.ws_col == 0) {
      return false;
    }

    rows = ws.ws_row;
    cols = ws.ws_col;

    return true;
#else // hard way
      // move cursor to the bot-right corner
      // "cursor forwad 999, cursur down 998" C & B means stop on meeting edges
      // . or you  maybe the freaking 9999 inches size screen editor?

    if (12 != write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12)) {
      return false;
    }

#endif
  }

  char read_key() {
    int cnt;
    char ch;
    while (-1 == (cnt = read(STDIN_FILENO, &ch, 1))) {
      if (cnt == -1 && errno != EAGAIN) {
        die("read");
      }
    }

    return ch;
  }
};

int main(int argc, char **argv) {
  try {
    Editor ed;
    ed.run();
  } catch (std::exception &e) {
    printf("%s\n", e.what());
    // print the erron string?
  }

  return 0;
}
