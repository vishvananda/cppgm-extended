// expect reject
// N3485 14.1p4: a bare unresolved auto non-type parameter is not the function trailing-return form.
template<auto Value> struct Bad {};
