// N3485 5.2.2p7,4.1p2: evaluated class glvalues initialize an ellipsis argument temporary.
// expect exit 0
int sink(...);struct A{A(const A&)=delete;};typedef char Check[sizeof(sink(*static_cast<A*>(0)))==sizeof(int)?1:-1];int main(){return sizeof(Check)!=1;}
