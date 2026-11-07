// N3485 13.3.3.1.2p3: a conversion-template specialization needs an Exact Match second sequence. expect exit 0
template<class T> struct I { typedef T type; };
int n=6;
struct X { template<class T=int*> operator typename I<T>::type() { return &n; } };
int main(){const int *p=X();return p==&n && *p==6?0:1;}
