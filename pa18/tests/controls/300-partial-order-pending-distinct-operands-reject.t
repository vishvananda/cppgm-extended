// expect reject
template<bool B,class T=void>struct enable_if{};
template<class T>struct enable_if<true,T>{typedef T type;};
template<class T>struct properties {enum{first=1,second=1};};
template<class F,class=void>struct select;
template<class T,class U>struct select<void(T,U),typename enable_if<(properties<T>::first!=0)>::type>{};
template<class T,class U>struct select<void(T,U),typename enable_if<(properties<U>::first!=0)>::type>{};
select<void(int,int)> ambiguous;
int main(){return 0;}
