struct Counter {
  int *p;
  Counter(int *q) : p(q) {}
  Counter(const Counter &other) : p(other.p) {}
  ~Counter() { ++*p; }
};

struct Hooks {
  Counter c;
  Hooks(int *p) : c(p) {}
};

Hooks make_hooks(int *p) { return Hooks(p); }
int read(const Hooks &h) { return *h.c.p; }

int main() {
  int destroyed = 0;
  int before = read(make_hooks(&destroyed));
  int after = destroyed;
  return (before == 0 && after == 1) ? 0 : 1;
}
