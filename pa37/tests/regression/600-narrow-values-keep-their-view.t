// A narrow value that a load or an operation left in the view of its type is
// compared and widened as it is. A sum, a product and a mask of narrow values
// read no view of their operands.
enum Kind : unsigned char { None, Leaf, Branch };
struct Item { Kind kind; signed char level; unsigned short tag; short delta; };
extern "C" __attribute__((noinline)) int is_branch(const Item* item) {
    return item->kind == Branch;
}
extern "C" __attribute__((noinline)) int below(const Item* item) {
    return item->level < -3;
}
extern "C" __attribute__((noinline)) long long widened(const Item* item) {
    return (long long)item->tag + item->delta;
}
extern "C" __attribute__((noinline)) void bump(Item* item, signed char step) {
    item->level = (signed char)(item->level + step);
    item->tag = (unsigned short)(item->tag * 3);
}
int main() {
    Item item = {Branch, -5, 65535, -2};
    if (!is_branch(&item) || !below(&item)) return 1;
    if (widened(&item) != 65533) return 2;
    bump(&item, 100);
    if (item.level != 95 || item.tag != 65533) return 3;
    bump(&item, 100);
    return item.level != -61 || item.tag != 65527;
}
