// Adopted CWG1696: constructor declaration eligibility versus body-definition demand.
// expect reject
struct Holder { const int& reference=42; }; int main() { Holder values[2]; }
