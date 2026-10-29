template<unsigned long N> struct Size { };
template<class T> void unused(T) {
    Size<sizeof(*1)> invalid;
}
int main() { return 0; }
