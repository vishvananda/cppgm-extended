int alive;
struct Owner {
 int n;
 explicit Owner(int x):n(x) { ++alive; }
 Owner(const Owner&)=delete;
 Owner(Owner&& other):n(other.n) { other.n=0; ++alive; }
 ~Owner(){--alive;}
};
int fail(){throw 7;}
int& fail_reference(){throw 8;}
template<class T> auto factory(T x) { return [value=x+2](auto y){return value+y;}; }
template<class T> auto nested(T x) { return [x](auto y){return [sum=x+y](auto z){return sum+z;};}; }
template<class... Ts> int expanded(Ts... xs) {
 int total=0;
 int each[]={0,(total += [copy=xs](auto delta){return copy+delta;}(1),0)...};
 return total+each[0];
}
int main(){
 int x=4;
 auto a=[x=x+1,&r=x](int y)mutable{r+=y;return x++;};
 if(a(3)!=5||a(2)!=6||x!=9)return 1;
 auto b=[x{3}](){return x;}; if(b()!=3)return 2;
 auto c=[=,x=x+1](){return x;}; if(c()!=10)return 3;
 auto f=factory(8);if(f(3)!=13)return 4;
 auto n=nested(2);if(n(3)(4)!=9)return 5;
 if(expanded(1,4,7)!=15)return 6;
 {
  Owner original(12);
  auto moved=[owner=static_cast<Owner&&>(original)](){return owner.n;};
  if(moved()!=12||original.n!=0||alive!=2)return 7;
  auto again=static_cast<decltype(moved)&&>(moved);
  if(again()!=12||alive!=3)return 8;
 }
 if(alive)return 9;
 try { auto broken=[owner=Owner(1),n=fail()](){return owner.n+n;}; return broken(); }
 catch(int code){if(code!=7||alive)return 10;}
 try { auto broken=[owner=Owner(1),&r=fail_reference()](){return owner.n+r;};return broken(); }
 catch(int code){if(code!=8||alive)return 11;}
 return alive;
}
