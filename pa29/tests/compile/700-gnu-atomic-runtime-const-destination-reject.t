struct Value { unsigned char bytes[17]; };
void f(const Value* p,Value* value){__atomic_store(p,value,__ATOMIC_SEQ_CST);}
