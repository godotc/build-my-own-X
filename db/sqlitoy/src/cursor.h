
#pragma once

#include <cstdint>
#include <memory>

struct Table;

struct Cursor {
  Table *table;
  uint32_t page_index;
  uint32_t cell_index;
  bool end_of_table;

  static std::unique_ptr<Cursor> table_start(Table *table);
  static std::unique_ptr<Cursor> table_end(Table *table);
  static std::unique_ptr<Cursor> table_find(Table *table, uint32_t key);
  static std::unique_ptr<Cursor>
  leaf_node_find(Table *table, uint32_t page_index, uint32_t key);

  char *value();
  void advance();
};
