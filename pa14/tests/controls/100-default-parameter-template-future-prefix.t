// CWG2082 andN3485 3.3.4p1: preserve each parameter-prefix lookup snapshot.
// expect exit 0
char value[31];
template<class T> int f(int n=sizeof(value),T value=sizeof(value)){return n+value;}
int main(){return f<long>()!=sizeof(::value)+sizeof(long);}
