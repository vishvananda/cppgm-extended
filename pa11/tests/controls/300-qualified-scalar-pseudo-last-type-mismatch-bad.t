// expect reject
// N3485 3.4.3p6, 5.2.4p1-p2: qualified scalar pseudo-destructor lookup and type identity.
struct Types{typedef int I; typedef double D;}; int main(){int v=7; (&v)->Types::I::~D();}
