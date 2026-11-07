// expect reject
// N3485 14.1p4: applying a pointer does not resolve a placeholder into a declared parameter type.
template<auto *Value> struct Bad {};
