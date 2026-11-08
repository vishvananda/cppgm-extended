// expect exit 0
struct Item { int n; Item(int value = 0) : n(value) {} };
struct Derived : Item { Derived() : Item(7) {} };
struct Wrapper { Wrapper(Item) {} };
int select(Item v) { return v.n; }
int select(const Wrapper&) { return -1; }
void mutate(Item& v) { v.n += 5; }
int main() {
    Item item(3); Derived derived;
    if (select({item}) != 3 || select({derived}) != 7) return 1;
    mutate({item});
    return item.n != 8;
}
