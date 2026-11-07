// expect exit 0
// N3485 3.4.3p6, 5.2.4p1-p2: qualified scalar pseudo-destructor lookup and type identity.
struct A { ~A(){} }; typedef A Alias; void destroy(A* p){p->Alias::~Alias();} int main(){return 0;}
