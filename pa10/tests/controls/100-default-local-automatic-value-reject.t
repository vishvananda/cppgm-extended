// expect reject
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int main(){int n=7;int target(int=n);}
