const char *padding[] = {
    "p00", "p01", "p02", "p03", "p04", "p05", "p06", "p07",
    "p08", "p09", "p10", "p11", "p12", "p13", "p14", "p15",
    "p16", "p17", "p18", "p19", "p20", "p21", "p22", "p23"
};
__attribute__((noinline)) unsigned choose(bool condition) {
    const char *first = condition ? "alpha" : "bravo";
    const char *second = condition ? "charlie" : "delta";
    return static_cast<unsigned char>(first[0]) * 256 + static_cast<unsigned char>(second[0]);
}
int main() {
    volatile bool yes = true, no = false;
    if (choose(yes) != 'a'*256u+'c' || choose(no) != 'b'*256u+'d') return 1;
    return padding[23][1] != '2';
}
