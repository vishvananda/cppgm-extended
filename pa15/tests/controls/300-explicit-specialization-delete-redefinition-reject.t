// CXX11-033; N3485 8.4.3p4 and 3.2p1. expect reject
// Repeating a deleted definition is not another first declaration.
template<class T> void action();
template<> void action<int>() = delete;
template<> void action<int>() = delete;
