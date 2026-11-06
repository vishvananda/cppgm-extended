// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
int probe(const long*);
char probe(long*);
int f(const long value,int n=sizeof(probe(static_cast<decltype(value)*>(0)))){return n;}
int main(){return f(0)!=sizeof(int);}
