// N3485 13.3.3.1.2p3: a conversion-template specialization needs an Exact Match second sequence. expect reject
template<class T> struct I { typedef T type; };
struct X { template<class T=long> operator typename I<T>::type() { return 4; } };
int f(){return X();}
