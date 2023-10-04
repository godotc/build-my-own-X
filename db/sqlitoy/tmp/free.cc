
#include <cstdint>
#include <cstdio>

struct Table {
  uint32_t num_rows;
  char *pages[100];

  Table() {
    for (uint32_t i = 0; i < 100; ++i) {
      pages[i] = nullptr;
    }
  }

  virtual ~Table() {
    for (auto page : pages) {
      delete page;
    }
  }
};

int main() {

  Table table;

  for (auto page : table.pages) {
    printf("%p\n", page);
  }
}
