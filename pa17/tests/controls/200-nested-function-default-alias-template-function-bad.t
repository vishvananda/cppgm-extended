// N3485 8.3.6p3, 14.1p8: defaults in a function type differ from defaults on actual parameters.
// expect reject
template<class T> using F=int(int = 0); F<int>* p; int main(){return 0;}
