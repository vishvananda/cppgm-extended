// expect exit 0
typedef decltype(sizeof(0)) Size;
namespace std {
template<class T> class initializer_list {
  const T* data;
  Size count;
public:
  constexpr initializer_list() : data(nullptr), count(0) {}
  constexpr const T* begin() const { return data; }
  constexpr Size size() const { return count; }
};
}
struct OnlyList {
  int n;
  constexpr OnlyList(std::initializer_list<int> values) : n(values.size()) {}
};
constexpr OnlyList empty{};
static_assert(empty.n == 0, "the empty list is one initializer_list argument");
OnlyList global{};
struct Both {
  int n;
  constexpr Both() : n(7) {}
  constexpr Both(std::initializer_list<int> values) : n(values.size() + 3) {}
};
constexpr Both chosen{};
constexpr Both nonempty{1, 2};
static_assert(chosen.n == 7, "an available default constructor wins");
static_assert(nonempty.n == 5, "nonempty lists select the list constructor");
int main() { return global.n; }
