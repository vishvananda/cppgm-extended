// N3485 5.2.3p1, 5.2.9p6, 12.2: discarded class results still run and die.
// expect exit 0
int alive, created, destroyed, calls;
struct Probe {
  Probe() { ++alive; ++created; }
  Probe(const Probe&) { ++alive; ++created; }
  ~Probe() { --alive; ++destroyed; }
};
Probe make() { ++calls; return Probe(); }
int main() {
  (void(make()));
  if (calls != 1 || alive != 0 || destroyed == 0 || destroyed != created) return 1;
  (void)make();
  static_cast<void>(make());
  return calls == 3 && alive == 0 && destroyed == created ? 0 : 2;
}
