// expect reject
// N3485 3.4.3p6, 5.2.4p1-p2: qualification scope is retained for scalar pseudo-destructors.
typedef int J; struct Types{typedef int I;}; int main(){int v=7; (&v)->Types::I::~J();}
