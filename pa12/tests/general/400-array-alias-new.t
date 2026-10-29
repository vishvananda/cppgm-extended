int made, destroyed;
int order[4];
struct Item { int value; Item() : value(++made) {} ~Item() { order[destroyed++] = value; } };
typedef Item Row[2];
typedef Row Grid[2];
int main() {
    Row* p = new Grid;
    if (made != 4 || p[1][1].value != 4) return 1;
    delete[] p;
    if (destroyed != 4) return 2;
    for (int i=0; i<4; ++i) if (order[i] != 4-i) return 3;
    return 0;
}
