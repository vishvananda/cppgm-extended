// Adopted CWG1696: constructor declaration eligibility versus body-definition demand.
// expect reject
struct Inner { const int& reference=42; }; struct Holder { Inner inner; }; int main() { Holder value; }
