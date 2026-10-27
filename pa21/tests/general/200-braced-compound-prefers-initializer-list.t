namespace std {
template<class T> class initializer_list {
  const T* data_;
  decltype(sizeof(0)) size_;
  constexpr initializer_list(const T* data, decltype(sizeof(0)) size) : data_(data), size_(size) {}
public:
  constexpr initializer_list() : data_(nullptr), size_(0) {}
  const T* begin() const { return data_; }
  const T* end() const { return data_ + size_; }
};
}
struct Sink {
  int value;
  Sink& operator+=(int) { value = -100; return *this; }
  Sink& operator+=(std::initializer_list<int> list) {
    for(int n : list) value += n;
    return *this;
  }
};
int main() {
  Sink sink{0};
  sink += {};
  Sink& same = sink += {1};
  sink += {2,3};
  return sink.value != 6 || &same != &sink;
}
