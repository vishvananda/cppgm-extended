struct Value { unsigned char bytes[17]; };
struct Other { unsigned char bytes[16]; };
void f(Value* p,Other* out){__atomic_load(p,out,__ATOMIC_SEQ_CST);}
