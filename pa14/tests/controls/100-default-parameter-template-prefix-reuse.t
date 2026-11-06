// CWG2082 andN3485 3.3.4p1: preserve each parameter-prefix lookup snapshot.
// expect exit 0
template<class T> int f(T value,T n=sizeof(value),T m=sizeof(value)){return n+m;}
int main(){return f(0L)!=2*sizeof(long);}
