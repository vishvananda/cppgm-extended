#include <typeinfo>

template<class T>
const std::type_info & make_info(T value)
{
  auto closure = [&value]() { return value; };
  return typeid(closure);
}

const std::type_info & use_info()
{
  return make_info(1);
}
