struct Value { int number; constexpr Value() : number(7) {} };
__constinit thread_local Value value;
int use() { static __constinit int number; return ++number + ++value.number; }
