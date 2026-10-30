int constructions;
int destructions;
struct flag {
  static const bool value = false;
  flag() { ++constructions; }
  ~flag() { ++destructions; }
  operator bool() const { return value; }
};
int main() {
  int selected = flag() ? 7 : 9;
  return selected != 9 || constructions != 1 || destructions != 1;
}
