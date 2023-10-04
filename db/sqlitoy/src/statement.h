#pragma once

#include <cstdint>
#include <memory>

struct InputBUffer;
struct Row;
struct Table;

enum EPrepareResult {
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID,

  PREPARE_INVALID_STATEMENT_ROW,
};

enum EStatementType {
  STATEMENT_INSERT,
  STATEMENT_SELECT,
};

enum EExecuteResult {
  EXECUTE_SUCCESS,

  EXECUTE_TABLE_FULL,
  EXECUTE_DUPLICATE_KEY,

  EXECUTE_ERROR_INSERT_INVALID_ROW,
};

struct Statement {
  EStatementType type;
  std::shared_ptr<Row> row_to_insert;
};

EPrepareResult prepare_statement(InputBUffer *buffer, Statement *statement);

EPrepareResult prepare_insert(InputBUffer *buffer, Statement *statement);

EExecuteResult exec_statement(Table *table, Statement *statement);
