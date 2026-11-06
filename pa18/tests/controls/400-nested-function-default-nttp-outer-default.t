// N3485 8.3.6p3, 14.1p8: defaults in a function type differ from defaults on actual parameters.
// expect exit 0
int target(int n){return n;} template<int F(int)=target>struct X{static int call(){return F(0);}}; int main(){return X<>::call();}
