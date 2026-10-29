// AUDIT: member-only operator ranking uses the lookup class; the selected call retains its declaring base.
int any_calls,key_calls,receivers,indices;
struct Key{const char* text;Key(const char* s):text(s){++key_calls;}};
struct Any{int value;Any(int n):value(n){++any_calls;}Any(const char*):value(-1){++any_calls;}};
struct Base{int data[2];int& operator[](int n){return data[n];}const int& operator[](int n)const{return data[n];}int& operator[](Key const& k){return data[k.text[0]=='x'];}const int& operator[](Key const& k)const{return data[k.text[0]=='x'];}};
struct Derived:Base{using Base::operator[];Any operator[](Any const&)const volatile{return Any(-1);}};
struct Further:Derived{};
Derived object;
Derived& receiver(){++receivers;return object;}
int index(){++indices;return 1;}
int main(){object.data[0]=7;object.data[1]=42;int& r=receiver()[index()];if(r!=42||receivers!=1||indices!=1)return 1;r=43;const Derived& c=object;if(c[1]!=43||object["x"]!=43||c["x"]!=43)return 2;Further f;f.data[1]=42;if(f[1]!=42)return 3;return any_calls!=0||key_calls!=2;}
