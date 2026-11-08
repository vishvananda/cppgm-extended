// expect reject
// Actual instantiation requires a valid friend declaration (14.5.3, 14.7.1).
template<class... Ts> struct T { friend Ts; };
T<int, char> object;
