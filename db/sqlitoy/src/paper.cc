
#include "paper.h"
#include "b_tree.h"
#include "consts.h"
#include <cstdint>
#include <stdexcept>

Paper::Paper(const char *file_name, uint32_t rows_per_page)
    : rows_per_page(rows_per_page) {
  open_file(this, file_name);

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
    pages[i] = nullptr;
  }

  if (num_pages == 0) {
    char *root_node = get_or_allocate_page(0);
    initialize_leaf_node(root_node);
  }
}

Paper::~Paper() {
  // printf("~Paper()\n");
  //  TODO: fallback to flush pages to file, now do it in upper Table class
  //  flush_pages_to_file(TABLE_MAX_PAGES)

  flush_pages_to_file(num_pages);

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    char *page = pages[i];
    if (page) {
      delete[] page;
      pages[i] = nullptr;
    }
  }
  int result = close(file_descriptor);
  if (-1 == result) {
    perror("error while closing db file");
  }
}

void open_file(Paper *p, const char *file_name) {
  if (!p) {
    throw std::runtime_error("null ptr of paper when open file");
  }

  p->file_descriptor =
      open(file_name,
           O_RDWR | O_CREAT // |create if no exist
           ,
           S_IWUSR | S_IRUSR //  User write permission| User  read permission
      );

  if (-1 == p->file_descriptor) {
    char buf[512];
    sprintf(buf, "Unable to open file: %s\n", file_name);
    perror(buf);
  }

  p->file_length = lseek(p->file_descriptor, 0, SEEK_END);
  if (p->file_length % PAGE_SIZE != 0) {
    throw std::runtime_error(
        "Db file is not a whole number of pages. Corrupt file.\n");
  }

  p->num_pages = p->file_length / PAGE_SIZE;
}

void Paper::flush_pages_to_file(uint32_t num_pages) {
  for (int i = 0; i < num_pages; ++i) {
    auto page = pages[i];
    if (page) {
      // flush whole page
      flush(i);
    }
  }

  // no need while using b_tree
  // uint32_t num_additional_rows = num_rows % rows_per_page;
  // if (num_additional_rows > 0) {
  //    uint32_t i    = num_full_pages;
  //    auto     page = pages[i];
  //    if (page) {
  //        // flush whole page
  //        flush(i);
  //    }
  //}

  printf("Num rows: %d, Num pages: %d\n", num_pages,
         num_pages /*+ num_additional_rows > 0*/);
}

void Paper::flush(uint32_t page_index) {
  if (!pages[page_index]) {
    perror("Try to flush a null page\n");
  }

  off_t offset = lseek(file_descriptor, page_index * PAGE_SIZE, SEEK_SET);
  if (-1 == offset) {
    perror("Error seeking file on flush paper");
  }

  ssize_t bytes_read = write(file_descriptor, pages[page_index], PAGE_SIZE);
  if (-1 == bytes_read) {
    perror("Error writing file on flush paper");
  }
}

char *Paper::get_or_allocate_page(uint32_t page_index) {
  if (page_index > TABLE_MAX_PAGES) {
    fprintf(stderr, "Page out of bound %d > %d\n", page_index, TABLE_MAX_PAGES);
    exit(EXIT_FAILURE);
  }

  char *page = pages[page_index];

  // missing cache, allocate memory / load from file
  if (!page) {
    page = pages[page_index] = new char[PAGE_SIZE];

    uint32_t num_max_pages = file_length / PAGE_SIZE;

    if (file_length % PAGE_SIZE /*!=0*/) {
      num_max_pages += 1;
    }

    if (page_index <= num_max_pages) {
      // retrieve this page and read one page data
      lseek(file_descriptor, page_index * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(file_descriptor, page, PAGE_SIZE);
      if (-1 == bytes_read) {
        fprintf(stderr, "Error  while reading file: %d\n", errno);
        exit(EXIT_FAILURE);
      }
    }

    pages[page_index] = page;

    if (page_index >= this->num_pages) {
      this->num_pages = page_index + 1;
    }
  }

  return pages[page_index];
}
