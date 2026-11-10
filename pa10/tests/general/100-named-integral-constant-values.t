// Known nonvolatile values may be read as constants; addresses, references
// and volatile reads retain their observable behavior. Expect exit 0.
namespace constants {
const int width = 8;
const bool never = false;
const bool always = true;
}
int touches;
int mutable_value = 3;
const int& reference_value = mutable_value;
volatile const int volatile_value = 19;

void touch(int* p) { ++touches; ++*p; }
int constant_read() { return constants::width; }
int false_choice(int x) { if (constants::never) { touch(&x); } return x; }
int true_choice(int x) { if (constants::always) { touch(&x); } return x; }
int volatile_control() { return volatile_value; }
int reference_control() { mutable_value = 11; return reference_value; }
const int* address_control() { return &constants::width; }

int main() {
  if (constant_read() != 8) return 1;
  if (false_choice(10) != 10 || touches != 0) return 2;
  if (true_choice(10) != 11 || touches != 1) return 3;
  if (volatile_control() != 19) return 4;
  if (reference_control() != 11) return 5;
  if (*address_control() != 8 || address_control() != &constants::width) return 6;
  return 0;
}
