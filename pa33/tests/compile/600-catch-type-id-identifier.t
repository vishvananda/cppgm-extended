#include <stdexcept>

int main() {
  try {
    throw std::out_of_range("x");
  } catch(std::out_of_range) {
    return 0;
  }
}
