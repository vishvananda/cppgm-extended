// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
long f(long value=decltype(value)()){return value;}
int main(){return f();}
