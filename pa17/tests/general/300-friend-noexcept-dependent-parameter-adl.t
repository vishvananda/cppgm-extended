namespace lib {
template<class T> struct Result {
  T& value() const { static T object; return object; }
  friend bool operator==(const Result& a, const Result& b) noexcept(noexcept(a.value() == b.value())) { return a.value() == b.value(); }
};
struct Value {
  Result<Value> force;
  friend bool operator==(const Value&, const Value&) noexcept { return true; }
};
}
int main() { lib::Result<lib::Value> result; return !(result == result); }
