// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
void f(long value=sizeof(value));
int main(){return 0;}
