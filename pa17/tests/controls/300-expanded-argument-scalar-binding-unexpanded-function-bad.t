// expect reject
// A singleton function parameter pack is still unexpanded before copying.
template<class... Types> struct Box {
  template<class... Args> int bad(Args... args) { return (args,0); }
};
int main() { Box<int> box; return box.bad(1); }
