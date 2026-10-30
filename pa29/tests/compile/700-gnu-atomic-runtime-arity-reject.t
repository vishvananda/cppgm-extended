struct Value { unsigned char bytes[17]; };
void f(Value* p,Value* out){__atomic_load(17,p,out,__ATOMIC_SEQ_CST);}
