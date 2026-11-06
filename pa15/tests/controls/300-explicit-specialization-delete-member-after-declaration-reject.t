// CXX11-033; N3485 8.4.3p4 and 14.7.3p16. expect reject
// The second explicit specialization declaration of an ordinary member is too late to delete it.
template<class T> struct Holder { void action(); };
template<> void Holder<int>::action();
template<> void Holder<int>::action() = delete;
