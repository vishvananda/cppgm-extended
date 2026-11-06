// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
int f(long (&value)[5],int n=sizeof(value)){return n;}
int main(){long data[5]={};return f(data)!=sizeof(data);}
