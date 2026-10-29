// AUDIT: member-only operator ranking uses the lookup class; the selected call retains its declaring base.
struct Any{Any(int){}};struct Payload{int value;};Payload fallback={7};
struct Pad{long long first,second;};struct Extra{long long words[2];};
struct Base{Payload payload;int& operator[](int){return payload.value;}const int& operator[](int)const{return payload.value;}Payload* operator->(){return &payload;}const Payload* operator->()const{return &payload;}};
struct Derived:Pad,Base{using Base::operator[];using Base::operator->;Any operator[](Any const&)const volatile{return Any(0);}Payload* operator->()const volatile{return &fallback;}};
struct Further:Extra,Derived{};
int main(){Further f;f.words[0]=5;f.words[1]=6;f.first=7;f.second=8;f.payload.value=42;if(static_cast<void*>(static_cast<Base*>(&f))==static_cast<void*>(&f))return 2;const Further& c=f;int& r=f[1];r=43;return f[1]!=43||c[1]!=43||f->value!=43||c->value!=43;}
