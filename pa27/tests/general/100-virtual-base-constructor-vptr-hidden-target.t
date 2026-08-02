struct V { virtual void probe() {} };
struct B : virtual V { B(); };
B::B() {}
struct Payload { virtual int read() { return 0; } };
struct Holder { Payload payload; };
struct D : Holder, B {};
int main() { D d; return d.payload.read(); }
