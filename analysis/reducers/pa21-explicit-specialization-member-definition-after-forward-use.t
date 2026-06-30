template<class T>
struct box {
  typedef T value_type;
  static void assign(value_type &, const value_type &);
};

box<char> * before = 0;

template<>
struct box<char> {
  typedef char value_type;
  static inline void assign(value_type & a, const value_type & b) { a = b; }
};

int main()
{
  char x = 0;
  box<char>::assign(x, 'a');
  return x == 'a' ? 0 : 1;
}
