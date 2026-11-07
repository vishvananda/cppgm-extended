// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect reject
int*p;constexpr int*q=p;int main(){return 0;}
