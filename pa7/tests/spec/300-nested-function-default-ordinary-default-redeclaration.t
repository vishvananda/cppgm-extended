// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect exit 0
int f(int = 0); int f(int n){return n;} int main(){return f();}
