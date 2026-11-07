// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect reject
void f();decltype(nullptr)n;constexpr int*p=(f(),n);int main(){return 0;}
