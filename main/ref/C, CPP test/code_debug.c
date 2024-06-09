#include <stdio.h>

#define haha(a) func(a)

int func(int a = 5);

int main()
{
  haha();
  return 1;
}

int func(int a)
{
  printf("a = %d\n", a);
  return 1;
}