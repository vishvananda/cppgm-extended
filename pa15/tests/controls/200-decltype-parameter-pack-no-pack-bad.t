// N3485 7.1.6.2 and 14.5.3: function parameter packs remain dependencies in decltype.
// expect reject
template<class... T> struct Tuple {};
Tuple<decltype((int)0)...> bad;
