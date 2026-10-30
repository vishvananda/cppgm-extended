struct flag {
  static const bool value = true;
  constexpr operator bool() const { return value; }
};
constexpr flag constant = {};
static_assert(constant ? true : false, "constant evaluation remains available");
int main() {
  flag value;
  return (value ? 7 : 9) != 7;
}
