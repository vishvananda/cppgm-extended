struct Point { int x; int y; };

constexpr Point p = {3, 4};
static_assert(p.x == 3, "");
static_assert(p.y == 4, "");

constexpr int arr[] = {10, 20, 30};
static_assert(arr[0] == 10, "");
static_assert(arr[1] == 20, "");
static_assert(arr[2] == 30, "");

static_assert("hello"[0] == 'h', "");
static_assert("hello"[4] == 'o', "");

int main() { return 0; }
