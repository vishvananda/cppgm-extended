// N3485 14.3/14.8.1p9; known extra explicit elements cannot be hidden by a residual expansion; expect reject
template<class T> struct Box {};
template<class... T> int inspect(Box<T...>*) { return sizeof...(T); }
int main() { return inspect<int, char>(0); }
