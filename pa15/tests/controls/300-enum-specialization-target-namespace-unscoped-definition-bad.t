// N3485 14.7.3p1/p4/p5: enum-specialization target eligibility.
// expect reject
namespace N{enum E:int;} template<>enum N::E:int{zero}; int main(){}
