// expect reject
struct Other {}; Other sample;
struct Item { Item() : decltype(sample)() {} };
int main() { Item i; }
