// N3485 8.3.6p3, 14.1p8: defaults in a function type differ from defaults on actual parameters.
// expect exit 0
int target(int n){return n;} int main(){auto p=new auto(target); int result=(*p)(0); delete p;return result;}
