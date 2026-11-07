// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int target(int n){return n;}int main(){const int n=7;int target(int=n);return target()!=7;}
