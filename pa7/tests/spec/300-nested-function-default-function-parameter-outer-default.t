// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect exit 0
int target(int n){return n;} int f(int cb(int)=target){return cb(0);} int main(){return f();}
