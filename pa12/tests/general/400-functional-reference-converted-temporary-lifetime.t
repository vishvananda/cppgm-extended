int alive;
int conversions;
struct Target {
  int value;
  Target(int n) : value(n) { ++alive; }
  Target(const Target& other) : value(other.value) { ++alive; }
  ~Target() { --alive; }
};
struct Proxy {
  operator Target() const { ++conversions; return Target(7); }
};
int main() {
  Proxy proxy;
  {
    typedef Target&& Reference;
    Target&& target = Reference(proxy);
    if (target.value != 7 || alive != 1 || conversions != 1) return 1;
  }
  return alive != 0 || conversions != 1;
}
