// Adopted CWG1696: constructor declaration eligibility versus body-definition demand.
// expect reject
struct Base { Base(int) {} }; struct Holder : Base { using Base::Base; const int& reference=42; }; int main() { Holder value(3); }
