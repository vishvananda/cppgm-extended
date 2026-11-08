// expect reject
// N3485 5.19: qualification conversion cannot make an ordinary stored pointer constant.
int object=42; int* pointer=&object; constexpr const int* result=pointer; int main() { return 0; }
