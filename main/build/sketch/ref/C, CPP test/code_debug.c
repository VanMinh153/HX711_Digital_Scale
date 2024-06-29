#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\ref\\C, CPP test\\code_debug.c"
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