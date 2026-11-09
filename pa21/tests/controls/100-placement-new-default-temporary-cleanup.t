// N3485 5.3.4p11, 12.2p3: allocation defaults live through the full expression.
// expect exit 0
typedef decltype(sizeof(0)) size_t;
alignas(16) unsigned char storage[128];
int live, released, constructed;
struct Token {
  Token() { ++live; }
  ~Token() { --live; }
};
struct Item {
  static void* operator new(size_t, int tag, const Token& token = Token()) noexcept {
    if (live != 1) return 0;
    return storage;
  }
  static void operator delete(void*, int, const Token&) noexcept { ++released; }
  Item() { ++constructed; throw 7; }
};
int fail() { throw 9; }
struct Other {
  static void* operator new(size_t, int extra = fail()) noexcept { return storage; }
};
int main() {
  try { new (1) Item; return 1; }
  catch (int e) { if (e != 7 || live != 0 || constructed != 1 || released != 1) return 2; }
  try { Token guard; new Other; return 3; }
  catch (int e) { if (e != 9 || live != 0) return 4; }
  return 0;
}
