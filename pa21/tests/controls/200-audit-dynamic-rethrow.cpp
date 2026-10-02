// AUDIT-ID: EH-RETHROW-DYNAMIC
// AUDIT-EXPECT: run
void rethrow(){throw;}int main(){try{try{throw 7;}catch(int){rethrow();}}catch(int n){return n!=7;}return 1;}
