#pragma once

namespace helper_inline_ns {
inline namespace v1 {

template<typename A, typename B>
struct pair
{
  A first;
  B second;
};

template<typename T>
struct vec
{
  T value;
};

}  // namespace v1
}  // namespace helper_inline_ns

bool accept_arg_ranges(
    helper_inline_ns::v1::vec<helper_inline_ns::v1::pair<unsigned long, unsigned long> > & ranges);
