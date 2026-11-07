// expect exit 0
// Adopted CWG2346/P0588R1: local variables may be named without odr-use in default arguments.
template<int N>struct X{int value;X():value(N){}};int main(){const int n=10;struct Local{int read(X<n> x=X<n>()){return x.value;}};Local a;return a.read()!=10;}
