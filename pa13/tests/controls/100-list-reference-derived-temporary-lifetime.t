// N3485 8.5.4p3, 8.5.3p5, 12.2p5: bind to the complete derived temporary,
// preserve virtual dispatch, and extend its lifetime only for a local reference.
// expect exit 0
int alive = 0;
int destroyed = 0;
struct Abstract {
  Abstract() { ++alive; }
  Abstract(const Abstract&) = delete;
  ~Abstract() { --alive; ++destroyed; }
  virtual int value() const = 0;
};
struct Concrete : Abstract { int value() const { return 7; } };
int read(const Abstract& value) { return alive == 1 ? value.value() : 0; }
int main() {
  int result = read({Concrete()});
  if (result != 7 || alive != 0 || destroyed != 1) return 1;
  {
    const Abstract& reference = { Concrete() };
    if (reference.value() != 7 || alive != 1 || destroyed != 1) return 2;
  }
  return alive == 0 && destroyed == 2 ? 0 : 3;
}
