#include <stdio.h>
#define haha() func(1)

int func(int a)
{
  printf("a = %d\n", a);
}

int main()
{
  haha();
  return 1;
}