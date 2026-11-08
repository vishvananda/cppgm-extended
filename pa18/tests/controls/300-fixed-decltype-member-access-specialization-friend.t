// expect exit 0
// CWG 1273 NAD: access may be instantiation-dependent without naming a parameter.
template<class T> struct Select;
class First { int value; friend struct Select<int>; } first;
class Second { int value; friend struct Select<char>; } second;
template<class T> struct Select {
 template<class U> decltype(first.value) get() { return 3; }
 template<class U> decltype(second.value) get() { return 7; }
};
int main() { return Select<int>().get<void>() == 3 && Select<char>().get<void>() == 7 ? 0 : 1; }
