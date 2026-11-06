// N3485 7.2p4, 7.1.6.3p2: qualified enum definitions and existing enum type uses remain valid.
// expect exit 0
struct A{enum E:int;}; enum A::E:int{zero}; int main(){return A::zero;}
