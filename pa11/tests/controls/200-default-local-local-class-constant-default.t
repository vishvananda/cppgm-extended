// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int main(){const int n=5;struct Local{int read(int x=n){return x;}};Local a;return a.read()!=5;}
