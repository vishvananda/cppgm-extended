// Failed construction destroys the argument temporary and releases storage.
long long storage[8];
int allocations;
int cleanup_order;

void* operator new(unsigned long) {
  ++allocations;
  return storage;
}
void operator delete(void*) noexcept {
  --allocations;
  cleanup_order = cleanup_order * 10 + 1;
}

struct argument {
  ~argument() noexcept { cleanup_order = cleanup_order * 10 + 2; }
};
struct item {
  item(const argument&) { throw 7; }
};

struct nullable_item {
  static void* operator new(unsigned long) noexcept { return nullptr; }
  static void operator delete(void*) noexcept { cleanup_order = 99; }
  nullable_item(const argument&) { throw 8; }
};

int main() {
  try { new item{argument()}; }
  catch (int value) {
    if (value != 7 || allocations != 0 || cleanup_order != 21) return 1;
    // A null allocation skips initialization and argument temporary cleanup.
    nullable_item* skipped = new nullable_item{argument()};
    return skipped != nullptr || allocations != 0 || cleanup_order != 21;
  }
  return 2;
}
