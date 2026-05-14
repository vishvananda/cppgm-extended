// VALIDATION: run-pass
// N3485 focus: 7.1.6.2 [dcl.type.simple]

#include "support.h"

int global_value = 0;

int main()
{
  decltype(global_value) a = 0;
  decltype((global_value)) b = global_value;

  static_assert(is_same<decltype(a), int>::value, "decltype(id-expression) should be plain type");
  static_assert(is_same<decltype(b), int &>::value, "decltype((lvalue)) should be lvalue reference");

  b = 5;
  return global_value == 5 ? 0 : 1;
}
