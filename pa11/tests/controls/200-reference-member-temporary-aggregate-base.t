// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect reject
struct Base { const int& reference; }; struct Holder : Base { Holder() : Base{42} {} };
