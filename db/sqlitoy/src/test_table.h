#pragma once

#include "b_tree.h"
#include "consts.h"
#include "table.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/core.h>
#include <memory>
#include <stdexcept>

#include "cursor.h"
#include "statement.h"

#define COLUMN_EMAIL_SIZE 32
#define COLUMN_USERNAME_SIZE 255

struct TestRow : public Row {
  uint32_t id;
  char usernmae[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];

  std::string to_string() override {
    return "{" + std::to_string(id) + ", " + usernmae + ", " + email + "}";
  }

  void serilaize(char *destination) override;

  void desrilaize(char *destination) override;
};

const uint32_t ID_SIZE = size_of_attribute(TestRow, id);
const uint32_t USERNAME_SIZE = size_of_attribute(TestRow, usernmae);
const uint32_t EMAIL_SIZE = size_of_attribute(TestRow, email);
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

/*
 * Leaf Node Body Layout
 */
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;

const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;

const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

namespace testtable {

inline uint32_t *leaf_node_cell(char *node, uint32_t cell_index) {
  return (uint32_t *)(node + LEAF_NODE_HEADER_SIZE +
                      cell_index * LEAF_NODE_CELL_SIZE);
}

inline uint32_t *leaf_node_key(char *node, uint32_t cell_index) {

  return leaf_node_cell(node, cell_index);
}

inline char *leaf_node_value(char *node, uint32_t cell_index) {
  char *cell = (char *)leaf_node_cell(node, cell_index);
  return cell + LEAF_NODE_KEY_SIZE;
}

uint32_t get_node_max_key(char *node) {
  switch (get_node_type(node)) {
  case NODE_INTERNAL:
    return *internal_node_key(node, *internal_node_num_keys(node) - 1);
  case NODE_LEAF:
    return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
  case NODE_ROOT:
    break;
  }
}

inline void print_constants() {
  printf("ROW_SIZE: %d\n", ROW_SIZE);
  printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
  printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
  printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
  printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
  printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

inline void print_leaf_node(char *node) {
  uint32_t num_cells = *leaf_node_num_cells(node);
  printf("leaf (size %d)\n", num_cells);
  for (uint32_t i = 0; i < num_cells; i++) {
    uint32_t key = *leaf_node_key(node, i);
    printf("  - %d : %d\n", i, key);
  }
}

inline void leaf_node_insert(Cursor *cursor, uint32_t key, Row *row_content) {
  char *node = cursor->table->get_or_allocate_page(cursor->page_index);

  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    throw std::runtime_error("Need to implement splitting a leaf node.\n");
  }

  if (cursor->cell_index < num_cells) {
    // mkae room for new cell
    for (uint32_t i = num_cells; i > cursor->cell_index; i--) {
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
             LEAF_NODE_CELL_SIZE);
    }
  }

  *(leaf_node_num_cells(node)) += 1;
  *(leaf_node_key(node, cursor->cell_index)) = key;
  row_content->serilaize(leaf_node_value(node, cursor->cell_index));
}

inline void leaf_node_split_and_insert(Cursor *cursor, uint32_t key,
                                       Row *row_value) {
  /*
   * Create a new node to move half cells over.
   * Insert the new value(row) in one of 2 nodes.
   * Update parent or create a new parent
   */

  char *old_node = cursor->table->get_or_allocate_page(cursor->page_index);
  uint32_t new_page_index = cursor->table->paper->get_unused_page_num();
  char *new_node = cursor->table->get_or_allocate_page(new_page_index);

  // copy origin to left and right
  for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; --i) {
    char *dest_node = {nullptr};
    if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
      dest_node = new_node;
    } else {
      dest_node = old_node;
    }

    uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
    char *dest = leaf_node_value(dest_node, index_within_node);

    if (i == cursor->cell_index) {
      row_value->serilaize(dest);
    } else if (i > cursor->cell_index) {
      memcpy(dest, leaf_node_value(old_node, i - 1), LEAF_NODE_CELL_SIZE);
    } else {
      memcpy(dest, leaf_node_value(old_node, i), LEAF_NODE_CELL_SIZE);
    }
  }

  // update cell count on both leaf nodes
  *leaf_node_num_cells(old_node) = LEAF_NODE_LEFT_SPLIT_COUNT;
  *leaf_node_num_cells(new_node) = LEAF_NODE_RIGHT_SPLIT_COUNT;

  // update root node;
  if (is_node_root(old_node)) {
    return create_new_root(cursor->table, new_page_index);
  } else {
    throw std::runtime_error("Need to implement updateing parent after split");
  }
}

}; // namespace testtable

struct TestTable : public Table {

  TestTable(const char *file_name)
      : Table(file_name, ROW_SIZE, ROWS_PER_PAGE) {}

  EExecuteResult exec_insert(Statement *statement) override {
    char *node = get_or_allocate_page(root_page_index);
    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS) {
      return EXECUTE_TABLE_FULL;
    }

    auto row = std::dynamic_pointer_cast<TestRow>(statement->row_to_insert);
    if (!row) {
      fprintf(stderr, "try to insert a none corresponing row\n");
      return EXECUTE_ERROR_INSERT_INVALID_ROW;
    }

    uint32_t key_to_insert = row->id;
    auto cursor = Cursor::table_find(this, key_to_insert);

    if (cursor->cell_index < num_cells) {
      uint32_t key_at_index =
          *testtable::leaf_node_key(node, cursor->cell_index);
      if (key_at_index == key_to_insert) {
        return EXECUTE_DUPLICATE_KEY;
      }
    }

    testtable::leaf_node_insert(cursor.get(), key_to_insert, row.get());

    return EXECUTE_SUCCESS;
  }

  EExecuteResult exec_select(Statement *statement) override {
    auto cursor = Cursor::table_start(this);

    TestRow row;
    while (!cursor->end_of_table) {
      row.desrilaize(cursor->value());
      printf("%s\n", row.to_string().c_str());
      cursor->advance();
    }
    return EXECUTE_SUCCESS;
  }
};

inline void TestRow::serilaize(char *destination) {
  memcpy(destination + ID_OFFSET, &id, ID_SIZE);
  strncpy(destination + USERNAME_OFFSET, usernmae, USERNAME_SIZE);
  memcpy(destination + EMAIL_OFFSET, email, EMAIL_SIZE);
}

inline void TestRow::desrilaize(char *destination) {
  memcpy(&id, destination + ID_OFFSET, ID_SIZE);
  memcpy(&usernmae, destination + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&email, destination + EMAIL_OFFSET, EMAIL_SIZE);
}
