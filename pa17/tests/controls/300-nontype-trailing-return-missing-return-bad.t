// expect reject
// N3485 8.3.5p2: this function type has no trailing return or body from which to determine a return type.
template<auto Fn()> struct Bad {};
