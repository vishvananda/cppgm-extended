#include <cstring>
#include <cstddef>
#include <type_traits>
static_assert(std::is_same<decltype(std::strlen("")), std::size_t>::value, "<cstring> strlen -> size_t");
