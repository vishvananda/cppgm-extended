// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int value(int n){return n;}int* pointer(int* p){return p;}int& ref(int& p){return p;}int main(){static int n=4;int value(int=n);int* pointer(int* = &n);int& ref(int& = n);if(value()!=4)return 1;n=7;return value()!=7 || pointer()!=&n || &ref()!=&n;}
