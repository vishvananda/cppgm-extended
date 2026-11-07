// expect exit 0
// N3485 14.7.1: completing the copied nested specialization does not instantiate unused member bodies.
template<class T> struct Outer { template<int N> struct Item {}; template<> struct Item<0> { template<class U> struct Nested { int read(){return sizeof(T)*100+sizeof(U);} int unused(){return sizeof(typename U::missing);} }; }; };
struct Good { typedef int missing; };
int main(){Outer<int>::Item<0>::Nested<int> a;Outer<char>::Item<0>::Nested<char> b;return a.read()!=sizeof(int)*100+sizeof(int)||b.read()!=101;}
