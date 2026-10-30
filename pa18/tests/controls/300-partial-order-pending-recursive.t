// expect exit 0
template<bool B,class T=void>struct enable_if{};
template<class T>struct enable_if<true,T>{typedef T type;};
template<bool B,class T=void>using enable_if_t=typename enable_if<B,T>::type;
template<class Impl,class T,class Props,class=void,class=void>struct call_traits{enum{valid=0};typedef T result_type;};
template<class Impl,class T,class P>struct call_traits<Impl,T,void(P)>{enum{valid=1};typedef T result_type;};
template<class Impl,class T,class P,class Q>struct call_traits<Impl,T,void(P,Q),enable_if_t<call_traits<Impl,T,void(P)>::valid != 0>,enable_if_t<call_traits<Impl,typename call_traits<Impl,T,void(P)>::result_type,void(Q)>::valid != 0>>{enum{valid=2};typedef T result_type;};
template<class Impl,class T,class P,class Q,class...R>struct call_traits<Impl,T,void(P,Q,R...),enable_if_t<call_traits<Impl,T,void(P)>::valid != 0>,enable_if_t<call_traits<Impl,typename call_traits<Impl,T,void(P)>::result_type,void(Q,R...)>::valid != 0>>{enum{valid=3};typedef T result_type;};
static_assert(call_traits<int,int,void(int,long)>::valid==2,"fixed");
static_assert(call_traits<int,int,void(int,long,char)>::valid==3,"variadic");
int main(){return 0;}
