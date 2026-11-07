// expect reject
// N3485 3.4.3p6, 5.2.4p1-p2: qualified scalar pseudo-destructor lookup and type identity.
typedef int I; int main(){int v=7; (&v)->I::~Missing();}
