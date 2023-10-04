

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
int main() {

  printf("%d\n", sizeof(void));
  void *ptr = malloc(20 * sizeof(int));

  printf("%p\n", ptr);
  printf("%p\n", ptr + 1);

  // int *p1 = ptr;
  // printf("%p\n", p1);
  // printf("%p\n", p1 + 1);

  return 0;
};
