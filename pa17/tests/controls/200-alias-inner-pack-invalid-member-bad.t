// Nested alias expansion boundary: N3485 14.5.3/14.5.7 and CWG1558.
template<class...> struct Pack {};
// The nonempty expansion must perform its member lookup.
// expect reject
template<class A, class... B> using Maybe = Pack<Pack<typename A::Member, B>...>;
struct Good { using Member = int; };
struct Bad {};
Maybe<Good, char>* valid;
Maybe<Bad, char>* invalid;
