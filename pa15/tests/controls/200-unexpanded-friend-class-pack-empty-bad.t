// expect reject
// Actual instantiation requires a valid friend declaration (14.5.3, 14.7.1).
template<class...> struct S {};
template<class... Ts> struct T { friend class S<Ts>; };
T<> object;
