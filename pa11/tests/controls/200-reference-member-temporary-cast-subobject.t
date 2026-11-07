// Adopted CWG1696/1815: temporary binding versus existing object/reference lifetime.
// expect reject
struct Value { int n; }; struct Holder { const int& reference; Holder() : reference(static_cast<const Value&>(Value{7}).n) {} };
