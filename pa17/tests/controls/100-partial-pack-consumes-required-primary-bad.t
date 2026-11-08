// expect reject
// CWG 1495: a partial specialization must be more specialized than its primary.
template<int I, class T, class... Rest> struct A;
template<class... T> struct A<0, T...>;
