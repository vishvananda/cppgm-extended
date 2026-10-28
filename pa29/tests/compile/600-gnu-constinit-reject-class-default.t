struct V{int n;V():n(3){}};template<int N>struct C{static __constinit V v;};template<int N>V C<N>::v;int main(){return C<2>::v.n;}
