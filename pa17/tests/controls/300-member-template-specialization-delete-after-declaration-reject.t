// CXX11-033; N3485 8.4.3p4 and 14.7.3. expect reject
// A member function template's explicit specialization has its own first declaration.
struct Caller { template<class T> void action(T); };
template<> void Caller::action<int>(int);
template<> void Caller::action<int>(int) = delete;
