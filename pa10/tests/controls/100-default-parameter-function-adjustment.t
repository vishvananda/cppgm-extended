// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
int target(int){return 0;}
int f(int value(int),int n=sizeof(value)){return n;}
int main(){return f(target)!=sizeof(&target);}
