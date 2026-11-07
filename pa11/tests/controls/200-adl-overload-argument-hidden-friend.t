// expect exit 0
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
namespace N{struct X{friend int h(int(*p)(int)){return p(4);}};} int g(int n){return n+6;} long g(N::X){return 0;} int main(){return h(g)!=10;}
