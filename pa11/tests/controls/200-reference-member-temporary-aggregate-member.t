// Adopted CWG1696/1815: reference-member temporary binding is ill-formed in constructor initialization; ordinary lifetime extension remains.
// expect reject
struct Inner { const int& reference; }; struct Holder { Inner inner; Holder() : inner{42} {} };
