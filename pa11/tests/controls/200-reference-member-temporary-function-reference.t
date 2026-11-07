// Adopted CWG1696/1815: temporary binding versus existing object/reference lifetime.
// expect exit 0
int value=3; const int& existing() { return value; } struct Holder { const int& reference; Holder() : reference(existing()) {} }; int main() { Holder holder; value=7; return holder.reference==7 && &holder.reference==&value ? 0 : 1; }
