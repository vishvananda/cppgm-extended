// expect exit 0
// N3485 3.4.2p2: union associated parameter/return entities of every overload.
int g(int n){return n+1;} long g(double){return 0;} int h(int(*p)(int)){return p(5);} int main(){return h(&g)!=6;}
