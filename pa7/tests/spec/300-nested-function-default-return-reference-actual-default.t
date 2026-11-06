// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect exit 0
int target(int n){return n;} int (&factory(int = 0))(int){return target;} int main(){return factory()(0);}
