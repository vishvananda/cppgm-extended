#pragma once

namespace helper_inline_ns {

inline namespace v1 {

template<class T>
struct vec {
  explicit vec(T init) : value(init) {}

  T value;
};

}  // namespace v1

}  // namespace helper_inline_ns

namespace using_namespace_vector_link {

int total(const helper_inline_ns::vec<int> & value);

}  // namespace using_namespace_vector_link
