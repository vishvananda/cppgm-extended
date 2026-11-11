#include <cstdio>
int alive;
int destroyed;
struct Record {
 int value;
 Record(int x = 1) noexcept : value(x) { ++alive; }
 ~Record() noexcept {
  --alive; destroyed += value;
  if (value == 5) std::printf("destroyed %d\n", destroyed);
 }
};
Record global(5);
__attribute__((noinline)) Record& instance() {
 static Record value(7);
 return value;
}
int main() {
 if (alive != 1) return 1;
 volatile unsigned count = 9;
 Record* array = new Record[count];
 if (alive != 10) return 2;
 delete[] array;
 if (alive != 1 || destroyed != 9) return 3;
 Record* single = new Record(3);
 if (single->value != 3 || alive != 2) return 4;
 delete single;
 if (alive != 1 || destroyed != 12) return 5;
 { Record local(2); if (alive != 2) return 6; }
 if (destroyed != 14) return 7;
 if (&instance() != &instance() || instance().value != 7 || alive != 2) return 8;
 return 0;
}
