// N3485 8.3.6p3, 14.1p8: defaults in a function type differ from defaults on actual parameters.
// expect exit 0
template<class T> using F=T(int); int target(int n){return n;} int f(F<int>* p=target){return p(0);} int main(){return f();}
