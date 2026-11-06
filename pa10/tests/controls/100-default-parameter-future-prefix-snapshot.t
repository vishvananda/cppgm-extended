// CWG2082 andN3485 3.3.4p1: preserve each parameter-prefix lookup snapshot.
// expect exit 0
char value[31];
int f(int n=sizeof(value),long value=sizeof(value)){return n+value;}
int main(){return f()!=sizeof(::value)+sizeof(long);}
