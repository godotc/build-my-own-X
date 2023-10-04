#pragma once

#include "b_tree.h"
#include "consts.h"
#include "paper.h"
#include "statement.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

struct Row {
  virtual std::vector<std::string> to_strings() { return {"none implemented"}; }
  virtual std::string to_string() = 0;

  virtual void serilaize(char *destination) = 0;
  virtual void desrilaize(char *destination) = 0;
};

struct Table {
  std::unique_ptr<Paper> paper;
  uint32_t root_page_index;

  uint32_t rows_per_page = {0};

  //---------------------

  Table(const char *file_name, uint32_t row_size, uint32_t rows_per_page) {
    paper.reset(new Paper(file_name, rows_per_page));

    this->root_page_index = 0;
    this->rows_per_page = rows_per_page;
  }

  void create_new_root(uint32_t right_childe_page_index) {
    char *root = get_or_allocate_page(root_page_index);
    char *right_child = get_or_allocate_page(right_childe_page_index);
    uint32_t left_child_index = paper->get_unused_page_num();
    char *left_child = get_or_allocate_page(left_child_index);

    // store old root to left child
    mempcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    // so root node -> new internal node(with pointers to l/r children without
    // value)
    initialize_internal_node(root);
    set_node_root(root, true);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = get_node_max_key(left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_num;
  }

  virtual ~Table() { printf("Table destruction: Flushing pages to file...\n"); }

  virtual EExecuteResult exec_insert(Statement *statement) = 0;
  virtual EExecuteResult exec_select(Statement *statement) = 0;

  char *get_or_allocate_page(uint32_t page_num) {
    return paper->get_or_allocate_page(page_num);
  }

private:
  // depercate
  char *row_slot(uint32_t row_num) {
    uint32_t page_num = row_num / rows_per_page;
    char *page = get_or_allocate_page(page_num);

    uint32_t row_offset = row_num % rows_per_page;
    uint32_t byte_offset = row_offset * rows_per_page;

    return page + byte_offset;
  }
};
