#include <stdio.h>

#define func_ref(a) func(a)

int func(int a = 5);

int main()
{
  func_ref(2);
  return 1;
}

int func(int a)
{
  printf("a = %d\n", a);
  return 1;
}