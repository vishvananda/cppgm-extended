// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect exit 0
struct Value { int n; }; struct Source { Value* pointer; operator Value&() { return *pointer; } }; struct Holder { const Value& reference; Holder(Source& input) : reference(input) {} }; int main() { Value value={3}; Source source={&value}; Holder holder(source); value.n=7; return holder.reference.n==7 && &holder.reference==&value ? 0 : 1; }
