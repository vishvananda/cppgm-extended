// expect exit 0
// N3485 8.3.5p2,14.1p4: form the member-function pointer type; no member-pointer value or runtime operation.
struct Owner;
template<auto (Owner::*Fn)() const -> int> struct MemberParameter {};
int main() { return 0; }
