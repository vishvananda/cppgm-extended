// expect exit 0
int run() {
    static int value = 3;
    const int& reference = value;
    auto read = []() { return reference; };
    auto address = []() { return &reference; };
    int first = read();
    value = 7;
    return first == 3 && read() == 7 && address() == &value ? 0 : 1;
}
int main() { return run(); }
