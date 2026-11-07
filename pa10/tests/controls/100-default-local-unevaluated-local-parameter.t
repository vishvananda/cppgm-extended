// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int target(int a,int b){return a+b;}int main(){int n[3];int target(int a=sizeof(n),int b=sizeof(a));return target()!=sizeof(n)+sizeof(int);}
