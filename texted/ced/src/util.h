#pragma once

#include <cstdio>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

namespace ced {
inline void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    throw std::runtime_error(s);
}

struct RawMode {
    termios origin_mode;

    RawMode()
    {
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
        mode.c_cc[VMIN]  = 0;
        mode.c_cc[VTIME] = 1;

        ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &mode);
        if (-1 == ret) {
            die("tcgetattr");
        }
    }
    ~RawMode()
    {
        printf("\r\nTurnoff raw mode..\r\n");
        int ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_mode);

        if (-1 == ret) {
            die("tcgetattr");
        }
    }
};
} // namespace ced
