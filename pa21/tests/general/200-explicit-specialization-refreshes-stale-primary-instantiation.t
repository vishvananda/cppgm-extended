typedef unsigned long size_t;

template<class T>
struct char_traits {
  typedef T wrong_type;
};

typedef char_traits<char16_t> early;

template<>
struct char_traits<char16_t> {
  typedef char16_t char_type;
  static int compare(const char_type*, const char_type*, size_t) noexcept;
};

inline int char_traits<char16_t>::compare(const char_type*, const char_type*, size_t) noexcept {
  return 0;
}

int main() { return 0; }
