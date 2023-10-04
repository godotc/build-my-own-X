#pragma once

#include "consts.h"
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>

struct Paper;
void open_file(Paper *p, const char *file_name);

struct Paper {
  int file_descriptor;
  uint32_t file_length;
  char *pages[TABLE_MAX_PAGES];
  uint32_t num_pages;

  uint32_t rows_per_page = {0};

  Paper(const char *file_name, uint32_t rows_per_page);

  ~Paper();

  void flush_pages_to_file(uint32_t num_rows);
  void flush(uint32_t page_index);

  char *get_or_allocate_page(uint32_t page_index);
  uint32_t get_unused_page_num() { return num_pages; }
};
