// Runtime conversion calls and scalar conditional slots survive unoptimized lowering.
struct flag {
  static const bool value = true;
  operator bool() const { return value; }
};
int main() {
  flag value;
  return (value ? 7 : 9) != 7;
}
