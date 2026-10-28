// Substituting the enclosing class must rebind a member template even
// while its own argument remains a parameter of a deeper template.
template<class T> struct Owner {
  struct Member {
    template<class Sequence, int Index> struct Iterator {
      template<class> struct Dereference;
      template<class Other> struct Dereference<Iterator<Other, 0>> {
        typedef T type;
        static type value() { return 7; }
      };
      template<class Other> struct Dereference<const Iterator<Other, 0>> {
        typedef const T type;
        static type value() { return 9; }
      };
    };
  };
};
struct First : Owner<int>::Member::Iterator<void, 0>::Dereference<
    Owner<int>::Member::Iterator<void, 0>> {};
struct Second : Owner<long>::Member::Iterator<char, 0>::Dereference<
    const Owner<long>::Member::Iterator<char, 0>> {};
static_assert(sizeof(First::type) == sizeof(int), "first owner captured");
static_assert(sizeof(Second::type) == sizeof(long), "second owner captured");
int main() { return First::value() != 7 || Second::value() != 9; }
