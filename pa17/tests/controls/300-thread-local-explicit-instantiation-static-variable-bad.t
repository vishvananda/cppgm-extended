// CWG1793/N3485 14.7.2: storage policy for explicit instantiation; expect reject
template<class T> struct Plain { static int value; };
template<class T> int Plain<T>::value=11;
template static int Plain<int>::value;
int main(){return 0;}
