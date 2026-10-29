struct Meta{typedef int type;};
Meta make_meta();
typedef decltype(make_meta()) Unknown;
struct Call{
  Unknown::type operator()()const{return 42;}
  Unknown::type operator[](int n)const{return n+1;}
  friend Unknown::type operator+(const Call&,int);
};
Unknown::type operator+(const Call& call,int n){return call()+n;}
int main(){Call call;return call()!=42||call[41]!=42||(call+1)!=43;}
