// expect reject
// CWG1360/P2448R2 does not make an uninitialized scalar read a constant expression.
struct Zero { int x; explicit Zero()=default; }; struct Bad { Zero z; constexpr Bad() {} };
static_assert(Bad().z.x==0,"user-provided outer constructor does not zero-initialize"); int main(){return 0;}
