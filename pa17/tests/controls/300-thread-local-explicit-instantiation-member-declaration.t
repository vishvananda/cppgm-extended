// CWG1793/N3485 14.7.2: storage policy for explicit instantiation; expect exit 0
template<class T> struct S { static thread_local const volatile int value; };
template<class T> thread_local const volatile int S<T>::value=7;
extern template thread_local const volatile int S<int>::value;
template thread_local const volatile int S<int>::value;
int main(){return S<int>::value!=7;}
