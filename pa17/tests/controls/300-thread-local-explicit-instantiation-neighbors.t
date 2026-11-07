// CWG1793/N3485 14.7.2: storage policy for explicit instantiation; expect exit 0
template<class T> struct S { static thread_local int value; };
template<class T> thread_local int S<T>::value=7;
template<> thread_local int S<int>::value=9;
template int S<short>::value;
template<class T> struct Plain { static int value; };
template<class T> int Plain<T>::value=11;
template int Plain<int>::value;
int main(){return S<int>::value!=9||S<short>::value!=7||Plain<int>::value!=11;}
