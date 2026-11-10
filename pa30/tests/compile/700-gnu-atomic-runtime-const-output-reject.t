struct Value { unsigned char bytes[17]; };
void f(Value* p,const Value* out){__atomic_load(p,out,__ATOMIC_SEQ_CST);}
