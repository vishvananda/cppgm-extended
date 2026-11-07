// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int target(int n){return n;}int main(){constexpr int n=9;int target(int=n);return target()!=9;}
