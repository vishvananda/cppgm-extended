template<class T> struct Box {};
template<class T> struct Other {};
Box(int) -> Other<int>;
