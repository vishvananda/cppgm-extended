// expect exit 0
int effects;
struct Object { int x, y, a[2]; };
int choose() { ++effects; return effects & 1; }
int Object::* member() { ++effects; return &Object::y; }
int run() {
    Object object = {3, 5, {7, 11}};
    return []() {
        (void)(choose() ? object.x : object.y);
        (void)object.a[effects++ & 1];
        (void)(object.*member());
        return effects;
    }();
}
int main() { return run() != 3; }
