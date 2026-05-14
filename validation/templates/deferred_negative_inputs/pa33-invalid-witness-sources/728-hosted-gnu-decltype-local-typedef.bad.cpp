template <typename T>
void f(T t) {
  typedef __decltype(*t) Ref;
  typedef __decltype__(*t) AltRef;
}

int main() {
  int x = 0;
  f(&x);
  return 0;
}
