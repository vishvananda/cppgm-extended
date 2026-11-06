// CXX11-033; N3485 8.4.3p4. expect reject
// An earlier ordinary definition is also an earlier declaration.
template<class T> int action();
template<> int action<int>() { return 3; }
template<> int action<int>() = delete;
