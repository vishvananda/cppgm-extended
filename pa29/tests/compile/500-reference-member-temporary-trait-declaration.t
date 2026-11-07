// Adopted CWG1696: constructor declaration eligibility versus body-definition demand.
struct Holder { const int& reference=42; }; struct Defaulted { Defaulted()=default; const int& reference=42; }; static_assert(__is_constructible(Holder), "declaration is constructible"); static_assert(__is_constructible(Defaulted), "defaulted declaration is constructible");
