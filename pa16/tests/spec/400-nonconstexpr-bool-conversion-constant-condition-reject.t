struct flag {
  static const bool value = true;
  operator bool() const { return value; }
};
static_assert(flag() ? true : false, "non-constexpr conversion is not a constant expression");
int main() { return 0; }
