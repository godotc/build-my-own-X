
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "input_buffer.h"
#include "meta_command.h"
#include "statement.h"
#include "table.h"
#include "test_table.h"

#define TO_STRING(x) #x

void main_loop(const char *filename) {
  std::unique_ptr<TestTable> table;
  table.reset(new TestTable(filename));

  InputBUffer *input_buffer = new InputBUffer;

  while (1) {
    print_prompt();
    read_input(input_buffer);

    // meta_command
    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_command(table.get(), input_buffer)) {
      case (META_COMMAND_SUCCESS):
        continue;
      case (META_COMMAND_UNRECOGNIZED_COMMAND):
        printf("Unrecognized cmd: '%s'. \n", input_buffer->buffer);
        continue;
      case META_COMMAND_EXIT:
        return;
      }
    }

    Statement statement;

    // sql syntax
    switch (prepare_statement(input_buffer, &statement)) {
    case PREPARE_SUCCESS:
      break;
    case PREPARE_UNRECOGNIZED_STATEMENT: {
      printf("Unrecognized keyword at start of : '%s'. \n",
             input_buffer->buffer);
      continue;
    }
    case PREPARE_SYNTAX_ERROR: {
      printf("Syntax error, could not parse statement: '%s'. \n",
             input_buffer->buffer);
      continue;
    }
    case PREPARE_STRING_TOO_LONG: {
      printf("String too long on some filed!\n");
      continue;
    }
    case PREPARE_INVALID_STATEMENT_ROW: {
      fprintf(stderr, "(maybe row casting): %s\n",
              TO_STRING(PREPARE_INVALID_STATEMENT_ROW));
      continue;
    }
    case PREPARE_NEGATIVE_ID: {
      printf("ID must be positive.\n");
      continue;
    }
    }

    // exic
    switch (exec_statement(table.get(), &statement)) {
    case EXECUTE_SUCCESS:
      printf("Executed.\n");
      break;
    case EXECUTE_TABLE_FULL:
      printf("Error: table full!\n");
      break;
    case EXECUTE_ERROR_INSERT_INVALID_ROW:
      printf("Error: %s\n", TO_STRING(EXECUTE_ERROR_INSERT_INVALID_ROW));
      break;
    case EXECUTE_DUPLICATE_KEY:
      printf("Error: Duplicated key\n");
      break;
    }
  }

  delete input_buffer;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s [table name]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  char *filename = argv[1];
  main_loop(filename);

  return 0;
}
