// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int n=6;int value(int x){return x;}int main(){extern int n;int value(int=n);return value()!=6;}
