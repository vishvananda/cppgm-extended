struct Incomplete;
template<class T> int capture() {
    struct Local : T { };
    typedef typename Local::type Captured;
    return sizeof(Captured);
}
int main() { return capture<Incomplete>(); }
