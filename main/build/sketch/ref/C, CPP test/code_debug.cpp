#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\ref\\C, CPP test\\code_debug.cpp"
#include <iostream>
#include <string>
using namespace std;

//_____________________________________________________________________________________________________________________
enum enum_t
{
  ENUM_1 = 1,
  ENUM_2 = 2,
  ENUM_3 = 3
};

int func(enum_t e) {
  printf("e = %d\n", e);
  return 1;
}

int main () {
  func(ENUM_3);
  return 1;
}

//_____________________________________________________________________________________________________________________

// #define func_ref(a) func(a)
// #define func_ref2(a, b) func2(a, b)

// int func(int a = 5);
// int func2(int a = 25 , int b = 50);

// int main()
// {
//   func_ref(29);
//   // func_ref2(19, 29);
//   return 1;
// }

// int func(int a)
// {
//   printf("a = %d\n", a);
//   return 1;
// }

// int func2(int a, int b)
// {
//   printf("a = %d\n", a);
//   printf("b = %d\n", b);
//   return 1;
// }