// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect reject
struct Value { Value(int) {} }; struct Holder { const Value& reference; Holder(int input) : reference(input) {} };
