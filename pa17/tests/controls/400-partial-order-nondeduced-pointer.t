// expect exit 0
template<class T,class U>struct choice;
template<class T,class U>struct choice<T,U*> {enum{value=1};};
template<class T>struct choice<T,typename T::type*> {enum{value=2};};
struct base {typedef int type;};
static_assert(choice<base,int*>::value==2,"pointer to member type");
int main(){return 0;}
