// N3485 14.5.3p5: a non-type parameter pack also needs an expansion.
// expect reject
template<int> struct Number {};
template<int... Values> using Bad = Number<Values>;
