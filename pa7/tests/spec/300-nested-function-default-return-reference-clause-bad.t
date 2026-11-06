// N3485 focus: 8.3.6p3: defaults belong to actual function declarations, not nested function types.
// expect reject
int (&factory(int = 0))(int = 1); int main(){return 0;}
