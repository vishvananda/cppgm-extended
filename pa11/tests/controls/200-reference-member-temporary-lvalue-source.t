// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect exit 0
struct Holder { const int& reference; Holder(const int& input) : reference(input) {} }; int main() { int value=3; Holder holder(value); value=7; return holder.reference==7 && &holder.reference==&value ? 0 : 1; }
