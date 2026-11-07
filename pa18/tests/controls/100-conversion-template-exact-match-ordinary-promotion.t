// N3485 13.3.3.1.2p3: a conversion-template specialization needs an Exact Match second sequence. expect exit 0
struct X { operator short(){return 5;} };
int main(){int n=X();return n==5?0:1;}
