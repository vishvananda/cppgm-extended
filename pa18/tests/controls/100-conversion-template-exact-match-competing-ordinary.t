// N3485 13.3.3.1.2p3: a conversion-template specialization needs an Exact Match second sequence. expect exit 0
template<class T> struct I { typedef T type; };
struct X { template<class T=short> operator typename I<T>::type() { return 3; } operator long(){return 7;} };
int main(){int n=X();return n==7?0:1;}
