// expect exit 0
// N3485 9.8: an instantiated local class may have ordinary data and static member functions.
template<class T> T f() { struct Local { T member; static T value() { return T(3); } }; Local object = {T(4)}; return object.member + Local::value(); }
int main() { return f<int>() != 7; }
