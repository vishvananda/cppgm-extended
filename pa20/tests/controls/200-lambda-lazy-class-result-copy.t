// N3485 5.1.2, 6.6.3, 12.8: deduced value results complete their class; expect exit 0
// A reference parameter need not instantiate a class template. A lambda's
// deduced value result does need its copy constructor and destructor.
int balance;
template<class T> struct Value {
  Value() { --balance; }
  Value(const Value&) { --balance; }
  ~Value() { ++balance; }
};
struct Tag {};
template<class T> struct Incomplete;
int main() {
  auto reference_only = [](const Incomplete<int>& value) -> const Incomplete<int>& {
    return value;
  };
  auto copy_value = [](const Value<Tag>& value) { return (value); };
  {
    Value<Tag> value;
    copy_value(value);
  }
  return balance != 0;
}
