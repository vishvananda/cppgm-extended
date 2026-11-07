// CWG1793/N3485 14.7.2: storage policy for explicit instantiation; expect exit 0
template<class T> struct S { static thread_local int value; };
template<class T> thread_local int S<T>::value=7;
template thread_local int S<int>::value;
int main(){S<int>::value+=2;return S<int>::value!=9;}
