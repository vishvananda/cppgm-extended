// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect exit 0
decltype(nullptr)n;static_assert(n?false:true,"");int main(){return 0;}
