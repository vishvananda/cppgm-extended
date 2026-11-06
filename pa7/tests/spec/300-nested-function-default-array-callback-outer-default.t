// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect exit 0
int f(int (*cb[1])(int)=0){return cb?1:0;} int main(){return f();}
