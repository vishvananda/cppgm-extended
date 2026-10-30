// expect reject
template<class T,class U=void>struct choice;
template<class T>struct choice<T,typename T::first>{enum{value=1};};
template<class T>struct choice<T,typename T::second>{enum{value=2};};
struct base {typedef void first;typedef void second;};
int value=choice<base>::value;
int main(){return 0;}
