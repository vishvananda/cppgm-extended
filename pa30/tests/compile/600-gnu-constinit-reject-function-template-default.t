struct Value { Value() {} };
template<class T> int use() { static __constinit T value; return 0; }
int main() { return use<Value>(); }
