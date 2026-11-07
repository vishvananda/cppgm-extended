// N3485 13.3.3.1.2p3: a conversion-template specialization needs an Exact Match second sequence. expect exit 0
template<class T> struct I { typedef T type; };
int n=8;
struct X { template<class T=int&> operator typename I<T>::type() { return n; } };
int main(){const int &r=X();return &r==&n && r==8?0:1;}
