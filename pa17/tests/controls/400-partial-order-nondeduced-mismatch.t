// expect exit 0
template<class Sig,class F>struct callable {};
template<class C,class Base,class F,class Enable=void>struct iface;
template<class R,class F,class Base,class Enable,class... T>
struct iface<callable<R(T...),F>,Base,F,Enable>{enum{value=1};};
template<class R,class F,class Base,class... T>
struct iface<callable<R(T...),F>,Base,F,typename Base::type>{enum{value=2};};
struct base {typedef int type;};
static_assert(iface<callable<void(),int>,base,int,void>::value==1,"member type does not match argument");
int main(){return 0;}
