// expect exit 0
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
namespace N{struct X{};int h(int(*p)(X,int)){return p(X(),8);}} template<class T>int g(N::X,T n){return n+1;} int main(){return h(g)!=9;}
