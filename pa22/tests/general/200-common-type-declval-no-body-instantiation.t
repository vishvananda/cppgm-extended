template<class T>
T&& __declval(int);

template<class T>
T __declval(long);

template<class T>
decltype(__declval<T>(0)) declval() {
  static_assert(!__is_same(T, T), "declval body");
}

template<class T, class U>
struct common_type {
  typedef decltype(true ? declval<T>() : declval<U>()) type;
};

typedef common_type<long long, int>::type X;

int main() {
  return 0;
}
