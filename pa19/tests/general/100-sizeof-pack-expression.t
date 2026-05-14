// HHC-126
template<class... I>
struct X {
  static constexpr int size() noexcept { return sizeof...(I); }
};

int main() { return X<int, char>::size() != 2; }
