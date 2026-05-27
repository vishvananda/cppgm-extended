#include <memory>
#include <string>
#include <unordered_map>

namespace {

struct Outer {
  enum E { A, B };
  std::allocator_traits<
      std::allocator<
          std::pair<unsigned long const,
                    std::unordered_map<std::string, Outer::E> > > >::pointer p;
};

}
