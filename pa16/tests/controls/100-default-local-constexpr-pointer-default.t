// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
int target=8;int main(){constexpr int* p=&target;struct Local{int read(int* q=p){return *q;}};Local a;return a.read()!=8;}
