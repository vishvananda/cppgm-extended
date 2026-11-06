// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect reject
typedef int (*F)(int = 0); int main(){return 0;}
