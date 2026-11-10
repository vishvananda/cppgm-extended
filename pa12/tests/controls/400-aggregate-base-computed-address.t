static int calls;
struct Pair { int x, y; };
Pair make_pair(int n) { ++calls; return {n, n + 1}; }
struct Parent { Pair point; int code; };
struct Child : Parent { Pair normal; };
void assign(Child *destination) { *destination = {{make_pair(7), 13}, make_pair(19)}; }
int main() {
    Child value{};
    assign(&value);
    if (calls != 2 || value.point.x != 7 || value.point.y != 8 || value.code != 13 || value.normal.x != 19 || value.normal.y != 20) return 1;
}
