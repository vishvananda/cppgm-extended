inline int value=7;
struct Item { inline static int count=3; };
int main() { return value+Item::count==10?0:1; }
