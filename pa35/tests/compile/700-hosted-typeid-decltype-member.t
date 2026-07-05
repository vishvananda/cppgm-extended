#include <cstddef>
#include <typeinfo>

typedef decltype(typeid(int).hash_code()) hash_type;

static_assert(sizeof(hash_type) == sizeof(std::size_t), "typeid hash_code type");

int main()
{
  return typeid(int).name()[0] == 0 ? 1 : 0;
}
