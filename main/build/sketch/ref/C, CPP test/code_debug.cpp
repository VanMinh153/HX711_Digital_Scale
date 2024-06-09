#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1 Scale\\main\\ref\\C, CPP test\\code_debug.cpp"
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