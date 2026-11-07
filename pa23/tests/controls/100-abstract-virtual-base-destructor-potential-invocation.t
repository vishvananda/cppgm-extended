// expect exit 0
// CWG1658: an abstract class never constructs or destroys its virtual bases.
struct B { private: ~B(){} }; struct A : virtual B { A(); virtual void f()=0; ~A(){} }; int main(){return 0;}
