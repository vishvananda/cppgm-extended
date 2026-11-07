// expect reject
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
namespace N{int g(int n){return n;} long g(double){return 0;} int h(int(*p)(int)){return p(0);}} using N::g; int main(){return h(g);}
