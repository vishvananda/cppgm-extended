template<class T> struct Base { };
template<class T> void unused() {
    struct Local : Base<int> { };
    typedef typename Local::missing Captured;
}
int main() { unused<int>(); return 0; }
