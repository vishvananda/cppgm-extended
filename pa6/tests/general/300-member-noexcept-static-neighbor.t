// CXX11-128: a static member exception specification has no implicit object.
int value;
void probe(int&) noexcept;
void probe(const int&) = delete;
struct Holder { static void inspect() noexcept(noexcept(probe(value))); };
int main() { return 0; }
