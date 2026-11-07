// expect exit 0
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
namespace N{struct X{};int h(int(*p)(int)){return p(5);}} int g(int n){return n+2;} template<class T>long g(N::X,T*){return 0;} int main(){return h(&g)!=7;}
