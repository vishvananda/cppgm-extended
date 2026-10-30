// expect exit 0
template<class T>struct wrap {};
template<class T,class U>struct choice;
template<class T,class U>struct choice<T,wrap<U>> {enum{value=1};};
template<class T>struct choice<T,wrap<typename T::type>> {enum{value=2};};
struct base {typedef int type;};
static_assert(choice<base,wrap<int>>::value==2,"nested member type");
int main(){return 0;}
