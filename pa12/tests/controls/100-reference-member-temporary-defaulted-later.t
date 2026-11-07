// Adopted CWG1696: constructor declaration eligibility versus body-definition demand.
// expect reject
struct Holder { Holder(); const int& reference=42; }; Holder::Holder()=default;
