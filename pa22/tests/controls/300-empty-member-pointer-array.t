// Empty member-pointer arrays contain null member pointers, whose ABI
// representation need not be all zero bytes. Expect exit 0.
struct Members {
    int before = 17;
    int Members::* pointers[8] = {};
    int after = 29;
};
int main() {
    Members m;
    if (m.before != 17 || m.after != 29) return 1;
    for (unsigned i = 0; i < 8; ++i) if (m.pointers[i] != nullptr) return 2;
    return 0;
}
