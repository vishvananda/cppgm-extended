// expect exit 0
struct Data { int value; };
struct Item { Data data; int calls; Data* get() { ++calls; return &data; } Item(); };
Item::Item() : data(), calls(0) { int(get()->value); }
int main() { Item i; return i.calls == 1 ? 0 : 1; }
