// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect reject
decltype(nullptr)&f();constexpr int*p=f();int main(){return 0;}
