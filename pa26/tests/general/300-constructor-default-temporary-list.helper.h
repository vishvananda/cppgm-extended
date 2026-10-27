namespace std {
template<class T> class initializer_list {
  const T* data_;
  decltype(sizeof(0)) size_;
  constexpr initializer_list(const T* data, decltype(sizeof(0)) size) : data_(data), size_(size) {}
public:
  constexpr initializer_list() : data_(nullptr), size_(0) {}
  constexpr decltype(sizeof(0)) size() const { return size_; }
};
}
extern int attempts, fail_at, live_alloc, live_value, made_alloc, made_value;
extern unsigned long long trace;
struct Alloc { int id; Alloc(); ~Alloc(); };
struct Value { int id; Value(int n, const Alloc& = Alloc()); ~Value(); };
void consume(std::initializer_list<Value> values);
