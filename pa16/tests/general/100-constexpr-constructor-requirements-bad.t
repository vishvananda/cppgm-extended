// CWG 1360/P2448R2 separate defaulted constructor eligibility from the
// actual constant initialization. B's default-initialization leaves x
// indeterminate: demanding the complete constant object must reject.
struct B { int x; };
struct D : B { int y; constexpr D() : y(0) {} };
constexpr D invalid = D();
int main() { return 0; }
