#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

struct InputBUffer {
    char  *buffer        = {nullptr};
    size_t buffer_length = {0};
    size_t input_length  = {0};

    ~InputBUffer() { delete buffer; }
};

inline void print_prompt() { printf("sqlitoy> "); }

inline void read_input(InputBUffer *input_buffer)
{
    size_t nbytes =
        getline(&input_buffer->buffer, &input_buffer->buffer_length, stdin);

    if (nbytes <= 0) {
        printf("Error when reading input");
        exit(EXIT_FAILURE);
    }

    // trim
    // std::string_view cmd(input_buffer->buffer);
    // const char *ws = " ";
    // cmd.remove_suffix(cmd.length() - cmd.find_last_not_of(ws) - 1);
    // cmd.remove_prefix(cmd.find_first_not_of(ws));

    // int x = 0;
    // for (int i = 0; i < cmd.size();) {
    //   input_buffer->buffer[x++] = cmd[i++];
    // }

    // for ingnore traling newline
    input_buffer->input_length       = nbytes;
    input_buffer->buffer[nbytes - 1] = '\0';
}
