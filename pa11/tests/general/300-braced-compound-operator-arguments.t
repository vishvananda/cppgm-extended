struct Sink {
  int value;
  Sink& operator+=(int n) { value += n; return *this; }
  Sink& operator-=(int n) { value -= n; return *this; }
  Sink& operator*=(int n) { value *= n; return *this; }
  Sink& operator/=(int n) { value /= n; return *this; }
  Sink& operator%=(int n) { value %= n; return *this; }
  Sink& operator<<=(int n) { value <<= n; return *this; }
  Sink& operator>>=(int n) { value >>= n; return *this; }
  Sink& operator&=(int n) { value &= n; return *this; }
  Sink& operator^=(int n) { value ^= n; return *this; }
  Sink& operator|=(int n) { value |= n; return *this; }
};
int lhs_calls, rhs_calls;
Sink& left(Sink& value) { ++lhs_calls; return value; }
int right() { ++rhs_calls; return 2; }
namespace adl {
struct Item { int value; };
Item& operator+=(Item& value, int n) { value.value += n; return value; }
enum class Bits { zero, one };
Bits& operator+=(Bits& value, int n) { value = static_cast<Bits>(static_cast<int>(value) + n); return value; }
}
struct Constant {
  int* value;
  const Constant& operator+=(int n) const { *value += n; return *this; }
};
int main() {
  Sink sink{10};
  Sink& same = left(sink) += {right()};
  sink -= {3}; sink *= {4}; sink /= {3}; sink %= {5};
  sink <<= {2}; sink >>= {1}; sink &= {6}; sink ^= {3}; sink |= {8};
  if (sink.value != 15 || &same != &sink || lhs_calls != 1 || rhs_calls != 1) return 1;
  adl::Item item{1}; item += {2};
  adl::Bits bits = adl::Bits::zero; bits += {1};
  int n = 0; const Constant constant{&n}; constant += {3};
  return item.value != 3 || bits != adl::Bits::one || n != 3;
}
