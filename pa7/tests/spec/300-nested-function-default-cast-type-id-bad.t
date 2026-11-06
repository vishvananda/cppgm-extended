// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect reject
int (*p)(int)=(int (*)(int = 0))0; int main(){return 0;}
