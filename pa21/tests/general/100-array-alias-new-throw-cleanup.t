int made, destroyed, released;
int order[2];
alignas(16) unsigned char storage[128];
struct Item {
    int value;
    Item() : value(++made) { if (value == 3) throw 42; }
    ~Item() { order[destroyed++] = value; }
    static void* operator new[](unsigned long) { return storage; }
    static void operator delete[](void*) { ++released; }
};
typedef Item Row[2];
typedef Row Grid[2];
int main() {
    int caught = 0;
    try { new Grid; return 1; }
    catch (int n) { caught = n; }
    return caught != 42 || made != 3 || destroyed != 2 || released != 1 || order[0] != 2 || order[1] != 1;
}
