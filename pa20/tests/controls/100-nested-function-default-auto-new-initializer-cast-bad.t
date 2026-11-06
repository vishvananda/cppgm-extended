// N3485 8.3.6p3, 14.1p8: defaults in a function type differ from defaults on actual parameters.
// expect reject
int main(){auto p=new auto((int (*)(int = 0))0); delete p;return 0;}
