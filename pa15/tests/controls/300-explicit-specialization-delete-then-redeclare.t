// CXX11-033; N3485 8.4.3p2,p4. expect exit 0
// A later declaration of the already-deleted specialization is permitted; it is not a use.
template<class T> void action();
template<> void action<int>() = delete;
template<> void action<int>();
int main() { return 0; }
