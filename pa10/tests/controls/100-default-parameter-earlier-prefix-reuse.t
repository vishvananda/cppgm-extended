// CWG2082 andN3485 3.3.4p1: preserve each parameter-prefix lookup snapshot.
// expect exit 0
int f(long value,long n=sizeof(value),long m=sizeof(value)){return n+m;}
int main(){return f(0)!=2*sizeof(long);}
