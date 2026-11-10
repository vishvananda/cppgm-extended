// The semantic value of a constexpr scalar is available to ordinary code.
// A reference still reads its current referent, and a static member's object
// expression still runs. Expect exit 0.
constexpr int scale(int x) { return x * 3; }
namespace constants {
constexpr int value = scale(7);
constexpr bool never = value == 0;
constexpr bool always = !never;
constexpr unsigned long mask = 0xfedcba9876543210UL;
}
int touches;
int mutable_value;
constexpr const int& reference_value = mutable_value;

void touch() { ++touches; }
int constant_read() { return constants::value; }
unsigned long mask_read() { return constants::mask; }
int false_choice() { if (constants::never) touch(); return touches; }
int true_choice() { if (constants::always) touch(); return touches; }
int reference_control() {
  mutable_value = 0;
  if (reference_value) touch();
  mutable_value = 1;
  if (reference_value) touch();
  return touches;
}
struct Static { static const int value = 7; };
Static object_effect() { touch(); return Static(); }
int static_member_control() { return object_effect().value; }
struct Member { const int value; Member(int x) : value(x) {} };
int member_control(int x) { Member object(x); return object.value; }

int main() {
  if (constant_read() != 21 || mask_read() != 0xfedcba9876543210UL) return 1;
  if (false_choice() != 0 || true_choice() != 1) return 2;
  if (reference_control() != 2) return 3;
  if (static_member_control() != 7 || touches != 3) return 4;
  if (member_control(31) != 31) return 5;
  return 0;
}
