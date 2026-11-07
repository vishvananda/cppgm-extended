// N3485 3.2,8.3.6p9,CWG2082/2083: consume actual conversions; retain default-parameter prohibition.
// expect reject
struct Object; void call(Object value = [] { return value; });
