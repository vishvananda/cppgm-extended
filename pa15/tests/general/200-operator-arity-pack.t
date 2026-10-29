template<class T> struct Box { int value; };
template<class T, class... U> int operator+(Box<T> x, U... values) { return x.value+sizeof...(values); }
int main() { Box<int> x={3}; return +x!=3 || x+1!=4; }
