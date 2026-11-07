// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect reject
struct Value { int n; }; struct Source { operator Value() { return Value{7}; } }; struct Holder { const Value& reference; Holder(Source& input) : reference(input) {} };
