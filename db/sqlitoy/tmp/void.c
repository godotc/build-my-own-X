

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
int main() {

  printf("%lu\n", sizeof(uintptr_t *));
  printf("%lu\n", sizeof(void *));
  printf("%d\n", sizeof(uintptr_t) == sizeof(void));

  void *ptr = malloc(20 * sizeof(uintptr_t));

  printf("%p\n", ptr);
  // here will add the address 1,
  // BUT in cpp, I cannot add 1 to the void ptr, AND I think it will add 8(size
  // of void ptr), instead of 1.
  // why?
  printf("%p\n", ptr + 1);

  int *p1 = ptr;
  printf("%p\n", p1);
  printf("%p\n", p1 + 1);

  return 0;
};
