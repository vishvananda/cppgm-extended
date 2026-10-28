struct Value {
 int value;
 Value(int v):value(v){}
 Value(const Value&)=delete;
 Value(Value&& other):value(other.value){other.value=-1;}
};
template<class T>struct Box {
 T value;
 Box(const T& v):value(v){}
 Box(T&& v):value(static_cast<T&&>(v)){}
};
Box<Value> local(){Value value(42);return value;}
Box<Value> parameter(Value value){return value;}
Box<Value> parenthesized(){Value value(43);return (value);}
int main(){return local().value.value!=42 || parameter(Value(44)).value.value!=44 || parenthesized().value.value!=43;}
