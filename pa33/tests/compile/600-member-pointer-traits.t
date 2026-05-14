#include <type_traits>

struct C
{
  int & f(const int &);
  int field;
};

using member_function_ptr = int & (C::*)(const int &);
using member_object_ptr = int C::*;

static_assert(std::is_default_constructible<member_function_ptr>::value,
              "member function pointers should be default constructible");
static_assert(std::is_default_constructible<member_object_ptr>::value,
              "member object pointers should be default constructible");
static_assert(std::is_trivially_copyable<member_function_ptr>::value,
              "member function pointers should be trivially copyable");
static_assert(std::is_scalar<member_function_ptr>::value,
              "member function pointers should be scalar");
static_assert(std::is_scalar<member_object_ptr>::value,
              "member object pointers should be scalar");

int main()
{
  member_function_ptr fn = nullptr;
  member_object_ptr field = nullptr;
  return fn == nullptr && field == nullptr ? 0 : 1;
}
