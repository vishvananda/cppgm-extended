// expect exit 0
// Identity to the enum itself remains better than underlying promotion.
enum E : char { e }; int pick(E) { return 1; } int pick(char) { return 2; }
int main() { return pick(e) == 1 ? 0 : 1; }
