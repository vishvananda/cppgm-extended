struct Value { int n; };
constexpr int Value::*field=&Value::n;
template<int Value::*Member>struct Data {};
Data<field> value;
int main() {}
