// expect exit 0
// N3485 8.5.4p5: an element with an NSDMI-backed array is constructed in list backing storage.
typedef decltype(sizeof(0)) Size; namespace std { template<class T> struct initializer_list { const T* data; Size count; constexpr Size size() const { return count; } constexpr const T* begin() const { return data; } }; }
enum E { Zero }; struct A { E e=Zero; }; struct B { A values[1]{}; }; struct V { int count; constexpr V(const std::initializer_list<B>& list):count(list.size()) {} }; constexpr V object{{}};
static_assert(object.count==1,"one backing element"); int main() { return object.count!=1; }
