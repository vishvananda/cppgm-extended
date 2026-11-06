// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect exit 0
using Callback=int(int); int f(Callback *cb=0){return cb?cb(0):0;} int main(){return f();}
