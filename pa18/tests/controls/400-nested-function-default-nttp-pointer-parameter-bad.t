// N3485 8.3.6p3, 14.1p8: defaults in a function type differ from defaults on actual parameters.
// expect reject
int target(int n){return n;} template<int (*F)(int = 0)>struct X{}; X<target> x; int main(){return 0;}
