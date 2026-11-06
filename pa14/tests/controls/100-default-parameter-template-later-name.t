// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
template<class T> int f(int n=sizeof(value),T value=T()){return n;}
int main(){return f<long>()!=sizeof(::value);}
