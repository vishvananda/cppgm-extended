// expect exit 0
struct Object { int value; operator int() const { return value; } };
Object object = { 7 };
template<const Object& Value> int read() { return Value.value; }
int main() { return read<object>() == 7 ? 0 : 1; }
