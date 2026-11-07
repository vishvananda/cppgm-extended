// N3485 14.5.3/14.8.1p9; the explicit nonempty pack leaves a possibly empty residual; expect exit 0
template<class T> struct Box {};
template<class... T> int inspect(Box<T...>*) { return sizeof...(T); }
int main() { Box<int>* argument = 0; return inspect<int>(argument) - 1; }
