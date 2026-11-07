// expect reject
// 5.2.4: a pseudo-destructor call requires a scalar object.
typedef int A[2];
void f(A *p) { p->A::~A(); }
