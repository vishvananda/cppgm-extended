// expect reject
// N3485 9.8p4 applies when a function template's local class is instantiated.
template<class T> int f() { struct Local { static T value; }; return 0; }
int main() { return f<int>(); }
