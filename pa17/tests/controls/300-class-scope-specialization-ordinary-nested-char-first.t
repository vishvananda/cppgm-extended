// expect exit 0
// CWG727: ordinary nested members inside a copied specialization own per-instance bodies.
template<class T> struct Outer { template<int N> struct Item {}; template<> struct Item<0> { struct Nested { int read(){return sizeof(T);} struct Deep { int read(){return sizeof(T)+2;} }; }; }; };
int main(){Outer<char>::Item<0>::Nested b; Outer<int>::Item<0>::Nested a; Outer<int>::Item<0>::Nested::Deep c; Outer<char>::Item<0>::Nested::Deep d; return a.read()!=sizeof(int)||b.read()!=sizeof(char)||c.read()!=sizeof(int)+2||d.read()!=sizeof(char)+2;}
