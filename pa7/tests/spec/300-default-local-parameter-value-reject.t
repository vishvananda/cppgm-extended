// VALIDATION: compile-fail
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int main(){int target(int n,int m=n);}
