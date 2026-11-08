struct Metadata { typedef int element; };
struct Solver { typedef Metadata NodeMetadata; };
template<class T> struct Graph { typedef typename T::NodeMetadata NodeMetadata; };
struct Derived : Graph<Solver> {};
struct Reader {
  int first(const Derived::NodeMetadata::element& value) { return value; }
};
int second(int prefix, const Derived::NodeMetadata::element& value) { return prefix + value; }
int main() { int value = 4; Reader reader; return reader.first(value) + second(3, value) == 11 ? 0 : 1; }
