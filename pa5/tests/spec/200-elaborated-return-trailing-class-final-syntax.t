// N3485 focus:8.4.1p1 syntax permits virt-specifier-seq;9.2p8 limits its semantic use.
// PA5 expects syntax acceptance;CXX11-044 owns required semantic rejection.
struct S{}; template<class T>auto f(T)->struct S final{return S{};} int main(){f(0);}
