// expect exit 0
// CWG295 applies after explicit substitution; this does not settle deduction in CWG1584.
int fn(int n){return n+2;}
template<class T>int invoke(const T* p){return p(3);}
template<class T>int indirect(const T** p){return (*p)(4);}
int main(){int(*p)(int)=fn;return invoke<int(int)>(&fn)!=5 || indirect<int(int)>(&p)!=6;}
