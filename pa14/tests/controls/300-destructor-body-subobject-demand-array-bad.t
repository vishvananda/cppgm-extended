// expect reject
// N3485 3.2p2,12.4p8: a defined destructor invokes non-variant subobject destructors.
template<class T> struct D { ~D(){ typename T::missing value; (void)value; } }; struct S { D<int> d[2]; S(); ~S(){} };
