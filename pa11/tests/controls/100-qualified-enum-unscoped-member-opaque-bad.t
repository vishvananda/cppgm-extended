// N3485 7.2p2, CWG1638: an ordinary opaque enum declaration cannot use a qualified name.
// expect reject
struct A{enum E:int;}; enum A::E:int; int main(){}
