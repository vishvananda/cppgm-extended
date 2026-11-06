// CXX11-033; N3485 8.4.3p4. expect reject
// Qualified declarations keep the same specialization identity.
namespace work { template<class T> void action(); }
template<> void work::action<int>();
template<> void work::action<int>() = delete;
