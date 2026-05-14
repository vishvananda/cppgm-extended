template<class T>
struct box {};

box(const char*) -> box<int>;
