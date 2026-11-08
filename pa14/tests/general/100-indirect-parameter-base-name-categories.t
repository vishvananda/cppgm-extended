struct Root {
  typedef long Word;
  int SizeType;
  struct ABI { int value; void set(int n) { value = n; } } TheCXXABI;
};
template<class T> struct Middle : T { typedef int Word; };
template<class T> struct Outer : Middle<T> {};
template<class T> struct More : Outer<T> {};
int choose(int) { return 1; }
int choose(long) { return 2; }
struct Derived : More<Root> {
  Derived() { Word value = 7; SizeType = value; TheCXXABI.set(13); }
  int check() { Word value = 0; return choose(value); }
};
int main() {
  Derived value;
  return value.SizeType == 7 && value.TheCXXABI.value == 13 && value.check() == 1 ? 0 : 1;
}
