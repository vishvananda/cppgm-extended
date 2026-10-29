// AUDIT: member-only operator ranking uses the lookup class; the selected call retains its declaring base.
struct Payload{int value;};Payload fallback={7};
struct Base{Payload payload;Payload* operator->(){return &payload;}const Payload* operator->()const{return &payload;}};
struct Derived:Base{using Base::operator->;Payload* operator->()const volatile{return &fallback;}};
struct Further:Derived{};
int main(){Derived d;d.payload.value=42;const Derived& c=d;if(d->value!=42||c->value!=42)return 1;Further f;f.payload.value=43;const Further& cf=f;return f->value!=43||cf->value!=43;}
