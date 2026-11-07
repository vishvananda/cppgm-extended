// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect exit 0
int result; struct Holder { int value; Holder() : value(7) { const int& local=42; result=local; } }; int main() { Holder holder; return result==42 && holder.value==7 ? 0 : 1; }
