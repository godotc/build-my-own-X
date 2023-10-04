#include "statement.h"
#include "fmt/core.h"
#include "input_buffer.h"

#include "test_table.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <numbers>
#include <regex>
#include <string>
#include <string_view>

#include <fmt/format.h>

void debug_match(std::string_view parttern, std::cmatch &match_result) {
  fmt::print("matching from re:\n "
             "\t'{}'\n"
             "are:\n",
             parttern);

  int i = 0;
  printf("\t%s\n", match_result[0].str().c_str());

  printf("\t{ ");
  for (auto res : match_result) {
    if (i < 1)
      ++i;
    else
      printf("%s, ", res.str().c_str());
  }
  printf("}\n");
}

EPrepareResult prepare_statement(InputBUffer *buffer, Statement *statement) {

  std::string_view cmd(buffer->buffer, buffer->buffer_length);

  printf("the statement brefore prepare(maybe trimed): %s\n", cmd.data());

  if (cmd.find("insert") == 0) {
    // fmt::println("--found insert prefix");
    return prepare_insert(buffer, statement);
  }

  if (cmd.find("select") != std::string_view::npos) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

EExecuteResult exec_statement(Table *table, Statement *statement) {

  // printf("%s\n", statement->row_to_insert->to_string().c_str());

  auto *table_to_insert = table;

  switch (statement->type) {
  case STATEMENT_INSERT:
    return table_to_insert->exec_insert(statement);
  case STATEMENT_SELECT:
    return table_to_insert->exec_select(statement);
  }
}

EPrepareResult prepare_insert(InputBUffer *buffer, Statement *statement) {
  statement->type = STATEMENT_INSERT;
  statement->row_to_insert.reset(new TestRow);

  auto row = std::dynamic_pointer_cast<TestRow>(statement->row_to_insert);
  if (!row) {
    return PREPARE_INVALID_STATEMENT_ROW;
  }
  char *keyword = strtok(buffer->buffer, " ");
  char *id_str = strtok(nullptr, " ");
  char *username = strtok(nullptr, " ");
  char *email = strtok(nullptr, " ");

  if (!id_str || !username || !email) {
    return PREPARE_SYNTAX_ERROR;
  }

  int id = atoi(id_str);
  if (id < 0) {
    return PREPARE_NEGATIVE_ID;
  }
  if (strlen(username) > COLUMN_USERNAME_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(email) > COLUMN_USERNAME_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }

  row->id = id;
  strcpy(row->usernmae, username);
  strcpy(row->email, email);

  return PREPARE_SUCCESS;

  {
    // the regex method
    // statement->type = STATEMENT_INSERT;
    // statement->row_to_insert.reset(new TestRow);

    // std::cmatch match_result;

    // if (auto row =
    //         std::dynamic_pointer_cast<TestRow>(statement->row_to_insert)) {

    //  std::string parttern("insert ([1-9]+) (.+) (.+)");
    //  std::regex re(parttern);

    //  const char *ptr = buffer->buffer;
    //  bool Ok =
    //      std::regex_match(ptr, ptr + buffer->buffer_length, match_result,
    //      re);
    //  if (!Ok) {
    //    fmt::println("re matching failed..");
    //    return PREPARE_SYNTAX_ERROR;
    //  }

    //  // DEBUG
    //  debug_match(parttern, match_result);

    //  row->id = std::stoi(match_result[1]);
    //  std::memcpy(row->usernmae, match_result[2].str().data(),
    //              sizeof(row->usernmae));
    //  std::memcpy(row->email, match_result[3].str().data(),
    //  sizeof(row->email));

    //  return PREPARE_SUCCESS;
    //}
  }
}
