// CWG1793/N3485 14.7.2: storage policy for explicit instantiation; expect exit 0
template<class T> thread_local int value=7;
template thread_local int value<int>;
int main(){value<int>+=2;return value<int>!=9;}
