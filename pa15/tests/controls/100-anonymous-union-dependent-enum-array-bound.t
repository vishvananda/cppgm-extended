// expect exit 0
// The anonymous union must resolve bounds in its enclosing specialization,
// including an enum that demands a preceding dependent static constant.
// A demanded anonymous union uses the enclosing specialization's enum value.
template<class T> struct storage {
  enum { capacity = 15 / sizeof(T) };
  union { T data[capacity + 1]; unsigned long spare; };
};
storage<char> object;
static_assert(sizeof(storage<char>) == 16, "enclosing enum bound");
int main() { object.data[15] = 7; return object.data[15] != 7; }
template<class T> struct dependent_storage {
  static const unsigned unit = sizeof(T);
  enum { capacity = 16 / unit };
  union { T values[capacity]; unsigned long spare; };
};
static_assert(sizeof(dependent_storage<int>) == 16, "bound can demand a preceding static constant");
static_assert(storage<short>::capacity == 7, "specialization-specific enum value");
