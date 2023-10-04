

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
struct A {
  int fd;
  A() {
    printf("C\n");
    fd = open("123456789", O_CREAT);
  }
  ~A() {
    printf("D\n");
    close(fd);
  }
};

int main() {
  A a;
  exit(-1);
  // throw std::runtime_error("abc");

  // fd cannot close, will leak?

  return 0;
}
