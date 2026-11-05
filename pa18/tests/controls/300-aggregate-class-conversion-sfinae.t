// N3485 14.8.2: aggregate member initialization participates in substitution.
// expect exit 0
template<class...> using Void = void;
template<class T> struct Array { T elements[1]; };
struct Reject { Reject(char*); };
struct Accept { Accept(int); };
template<class T, class = void> struct CanInitialize { enum { value = false }; };
template<class T> struct CanInitialize<T, Void<decltype(Array<T>{{42}})>> {
  enum { value = true };
};
static_assert(!CanInitialize<Reject>::value, "invalid member conversion rejects the specialization");
static_assert(CanInitialize<Accept>::value, "valid member conversion remains viable");
static_assert(CanInitialize<int>::value, "scalar member conversion remains viable");
template<class T, class = void> struct Excess { enum { value = false }; };
template<class T> struct Excess<T, Void<decltype(Array<T>{{1, 2}})>> {
  enum { value = true };
};
static_assert(!Excess<int>::value, "excess array elements reject the specialization");
int main() { return 0; }
