template<unsigned long N> struct Size { };
int runtime();
template<class T> void unused(T) {
    Size<runtime()> invalid;
}
int main() { return 0; }
