// expect reject
typedef unsigned long Size;
template<class T,class U>struct cross;
template<class T,Size N,Size M>struct cross<T[N],T[M]>{};
template<class T,class U,Size N>struct cross<T[N],U[N]>{};
cross<int[3],int[3]> value;
int main(){return 0;}
