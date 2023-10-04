#include "cursor.h"
#include "b_tree.h"
#include "table.h"
#include "test_table.h"
#include <algorithm>
#include <cstdint>
#include <fmt/core.h>
#include <memory>
#include <stdexcept>

std::unique_ptr<Cursor> Cursor::table_start(Table *table) {
  std::unique_ptr<Cursor> cs(new Cursor);
  cs->table = table;
  cs->page_index = table->root_page_index;
  cs->cell_index = 0;

  char *root_node = table->get_or_allocate_page(table->root_page_index);
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  cs->end_of_table = (num_cells == 0);

  return cs;
}

std::unique_ptr<Cursor> Cursor::table_end(Table *table) {
  std::unique_ptr<Cursor> cs(new Cursor);
  cs->table = table;
  cs->page_index = table->root_page_index;

  char *root_node = table->get_or_allocate_page(table->root_page_index);
  cs->cell_index = *leaf_node_num_cells(root_node);

  cs->end_of_table = true;

  return cs;
}

std::unique_ptr<Cursor> Cursor::table_find(Table *table, uint32_t key) {
  auto root_page_num = table->root_page_index;
  char *root_node = table->get_or_allocate_page(root_page_num);

  if (get_node_type(root_node) == NODE_LEAF) {
    return Cursor::leaf_node_find(table, root_page_num, key);
  }

  throw std::runtime_error("Need to implement searching an internal node\n");
}

std::unique_ptr<Cursor>
Cursor::leaf_node_find(Table *table, uint32_t page_index, uint32_t key) {
  char *node = table->get_or_allocate_page(page_index);
  uint32_t num_cells = *leaf_node_num_cells(node);

  std::unique_ptr<Cursor> cs(new Cursor);
  cs->table = table;
  cs->page_index = page_index;

  // binary search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;

  while (one_past_max_index != min_index) {
    uint32_t index = min_index + ((one_past_max_index - min_index) >> 1);
    uint32_t key_of_this_index = *testtable::leaf_node_key(node, index);
    if (key == key_of_this_index) {
      cs->cell_index = index;
      return cs;

    } else if (key < key_of_this_index) {
      min_index = index + 1;
    } else {
      one_past_max_index = index;
    }
  }

  cs->cell_index = min_index;
  return cs;
}

char *Cursor::value() {
  char *page = table->get_or_allocate_page(page_index);

  return testtable::leaf_node_value(page, cell_index);
}

void Cursor::advance() {
  auto *node = table->get_or_allocate_page(cell_index);
  ++cell_index;
  if (cell_index >= *leaf_node_num_cells(node)) {
    end_of_table = true;
  }
}
