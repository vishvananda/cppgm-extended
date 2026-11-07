// expect exit 0
// Valid trailing-return slice of LLVM Clang's SemaCXX/pr30559.cpp.
// Upstream revision: 2078da43e25a4623cab2d0d60decddf709aaea28.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// The original's unrelated IFNDR template is deliberately absent.
template <typename T>
auto foo(T x) -> decltype((x == nullptr), *x) {
  return *x;
}

int main() {
  int value = 42;
  int &reference = foo(&value);
  return &reference == &value && reference == 42 ? 0 : 1;
}
