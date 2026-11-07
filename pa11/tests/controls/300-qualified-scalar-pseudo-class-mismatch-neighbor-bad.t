// expect reject
// N3485 3.4.3p6, 5.2.4p1-p2: qualified scalar pseudo-destructor lookup and type identity.
struct A{~A(){}}; struct B{~B(){}}; void destroy(A*p){p->B::~B();} int main(){return 0;}
