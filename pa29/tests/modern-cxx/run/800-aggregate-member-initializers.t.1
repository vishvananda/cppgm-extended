struct Record { int value = 7; int remainder = value + 3; };
constexpr Record constant{9};
static_assert(constant.value == 9 && constant.remainder == 12, "default member sees explicit earlier value");
struct Method {
  int first = 2;
  int last = twice();
  int twice() const { return first * 2; }
};
template<class T> struct Optional { int value = T::missing; };
Optional<int> explicit_member{3};
struct Reference { int value = 7; int& alias = value; };
unsigned initialized;
int next() { ++initialized; return 3; }
struct Counted { int first = next(); int second = next(); };
Record make() { return {42}; }
int main() { Record first{2}; Record second{}; Record third = make();
 Method method{6}; Reference reference{9}; reference.alias = 11; Counted counted{5};
 return reference.value != 11 || counted.second != 3 || initialized != 1 || method.last != 12 || explicit_member.value != 3 || first.value != 2 || first.remainder != 5 || second.value != 7 || second.remainder != 10 || third.remainder != 45; }
