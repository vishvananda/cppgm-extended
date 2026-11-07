// N3485 14.7.3p1/p4/p5: enum-specialization target eligibility.
// expect reject
struct A{enum E:int;}; template<>enum A::E:int; int main(){}
