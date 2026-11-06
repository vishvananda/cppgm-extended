// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
int main(){auto f=[](long value,int n=sizeof(value)){return n;};return f(0)!=sizeof(long);}
