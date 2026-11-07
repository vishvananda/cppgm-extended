// CWG1793/N3485 14.7.2: storage policy for explicit instantiation; expect exit 0
int constructions;
struct Object {int value;Object():value(9){++constructions;}~Object(){--constructions;}};
template<class T>struct S{static thread_local Object value;};
template<class T>thread_local Object S<T>::value;
template thread_local Object S<int>::value;
int main(){Object& a=S<int>::value;Object& b=S<int>::value;return &a!=&b||constructions!=1||a.value!=9;}
