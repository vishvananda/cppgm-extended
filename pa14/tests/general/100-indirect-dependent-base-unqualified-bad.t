template<class T> struct Middle : T {};
template<class T> struct Derived : Middle<T> {
  void use() { Unknown = 1; }
};
int main() { return 0; }
