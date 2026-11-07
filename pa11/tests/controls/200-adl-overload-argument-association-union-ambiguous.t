// expect reject
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
namespace N{struct X{};int h(int(*)(int));} namespace M{struct X{};int h(int(*)(int));} int g(int);long g(N::X);long g(M::X);int main(){return h(g);}
