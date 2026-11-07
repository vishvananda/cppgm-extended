// Adopted CWG1696: constructor declaration eligibility versus body-definition demand.
// expect exit 0
struct Holder { const int& reference=42; }; static_assert(noexcept(Holder()), "only the constructor declaration is needed"); int main() { return 0; }
