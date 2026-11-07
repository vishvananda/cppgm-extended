// expect exit 0
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
namespace N{struct X{};int h(int(*p)(int)){return p(3);}} int g(int n){return n+4;} long g(N::X){return 0;} int main(){return h(g)!=7;}
