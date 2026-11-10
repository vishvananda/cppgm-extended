template<class T> struct Box { T value; Box(T x):value(x){} };
Box(int)->Box<long>;
int main() { Box value(7); return sizeof(value.value)==sizeof(long) && value.value==7?0:1; }
