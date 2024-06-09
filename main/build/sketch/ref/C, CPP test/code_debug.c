#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1 Scale\\main\\ref\\C, CPP test\\code_debug.c"
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