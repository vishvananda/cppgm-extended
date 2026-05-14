namespace N {

template<typename T>
struct Box {
  using value_type = T;

private:
  static constexpr int min_alignment =
      (sizeof(T) & (sizeof(T) - 1)) || sizeof(T) > 16 ? 0 : sizeof(T);
  static constexpr int alignment =
      min_alignment > alignof(T) ? min_alignment : alignof(T);
  alignas(alignment) T value;
};

template<typename T>
using diff_t = typename Box<T>::difference_type;

template<typename I>
inline I fetch_add_explicit(Box<I> * box, diff_t<I> offset, int mode) noexcept
{
  return box->fetch_add(offset, mode);
}

}

int main()
{
  return 0;
}
