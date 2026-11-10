struct Value { int number; constexpr Value() : number(3) {} };
__constinit Value value;
__constinit int copied = value.number;
