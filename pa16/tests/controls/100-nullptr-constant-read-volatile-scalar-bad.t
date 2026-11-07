// Adopted CWG2907 and2140: nullptr_t conversion fetches no stored value.
// expect reject
volatile int n=0;constexpr int v=n;int main(){return 0;}
