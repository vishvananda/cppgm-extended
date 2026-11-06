// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect exit 0
int target(int n){return n;} struct X{int (*f(int = 0))(int){return target;}}; int main(){X x;return x.f()(0);}
