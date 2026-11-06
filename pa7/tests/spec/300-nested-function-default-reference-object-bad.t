// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect reject
int f(int); int (&r)(int = 0)=f; int main(){return 0;}
