// CWG1793/N3485 14.7.2: storage policy for explicit instantiation; expect reject
template<class T>int f(){return 7;}
template inline int f<int>();
int main(){return 0;}
