// expect exit 0
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
namespace N{struct X{};int h(int(*p)(int)){return p(2);}} int g(int n){return n+7;} N::X g(double){return N::X();} int main(){return h(&g)!=9;}
