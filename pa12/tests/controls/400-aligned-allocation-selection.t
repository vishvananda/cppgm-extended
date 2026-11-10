typedef decltype(sizeof(0)) Size;
namespace std { enum class align_val_t : Size {}; }
alignas(64) unsigned char storage[128];
int allocations, deallocations, seen_alignment;
struct alignas(64) Cell {
  int value;
  static void* operator new(Size, std::align_val_t alignment) {
    ++allocations;
    seen_alignment = static_cast<int>(alignment);
    return storage;
  }
  static void operator delete(void* pointer, std::align_val_t alignment) noexcept {
    if (pointer == storage && static_cast<int>(alignment) == 64) ++deallocations;
  }
};
int main() {
  Cell* cell = new Cell{37};
  int value = cell->value;
  delete cell;
  return value == 37 && allocations == 1 && deallocations == 1 && seen_alignment == 64 ? 0 : 1;
}
