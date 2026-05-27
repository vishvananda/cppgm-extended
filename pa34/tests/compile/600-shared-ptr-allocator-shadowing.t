#include <memory>
#include <mutex>

void f() {
  auto p = std::make_shared<std::mutex>();
}
