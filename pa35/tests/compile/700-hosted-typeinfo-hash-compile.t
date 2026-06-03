#include <typeinfo>
#include <cstddef>
#include <type_traits>
static_assert(std::is_same<decltype(typeid(int).hash_code()), std::size_t>::value,
              "std::type_info::hash_code() -> std::size_t");
int main()
{
  const std::type_info & int_type = typeid(int);
  if(int_type == typeid(double)) { return 1; }
  if(!(int_type == typeid(int))) { return 2; }
  if(int_type.hash_code() != typeid(int).hash_code()) { return 3; }
  return 0;
}
