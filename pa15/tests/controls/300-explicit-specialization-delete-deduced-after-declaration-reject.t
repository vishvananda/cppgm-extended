// CXX11-033; N3485 8.4.3p4 and 14.7.3p10. expect reject
// Deducing the specialization's arguments from its type does not erase its declaration.
template<class T> void action(T);
template<> void action(int);
template<> void action(int) = delete;
