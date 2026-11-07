// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect exit 0
int live; struct Value { int n; Value(int input) : n(input) { ++live; } ~Value() { --live; } }; struct Aggregate { const Value& reference; }; int main() { { Aggregate value={Value(7)}; if(live!=1 || value.reference.n!=7) return 1; } return live; }
