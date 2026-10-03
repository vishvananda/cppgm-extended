// AUDIT-ID: BACKEND
// AUDIT-EXPECT: run
// Dynamic rethrow is accepted; native same-function nested-handler dispatch remains open.
void rethrow(){throw;}int main(){try{try{throw 7;}catch(int){rethrow();}}catch(int n){return n!=7;}return 1;}
