// CXX11-033; N3485 8.4.3p4 and 14.7.3p16. expect reject
// Both enclosing template heads select the same previously declared specialization.
template<class T> struct Caller { template<class U> int action(U); };
template<> template<> int Caller<int>::action<double>(double);
template<> template<> int Caller<int>::action<double>(double) = delete;
