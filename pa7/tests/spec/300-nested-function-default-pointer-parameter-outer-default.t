// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect exit 0
int f(int (*cb)(int)=0){return cb?cb(0):0;} int main(){return f();}
