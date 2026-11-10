struct Value { unsigned long long first,second; };
Value f(Value* p){return __atomic_load_n(p,__ATOMIC_SEQ_CST);}
