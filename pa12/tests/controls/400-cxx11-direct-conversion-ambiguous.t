struct Basis {}; struct Target { Target(const Basis&); }; struct Variant { operator Basis() const; operator Target() const; }; Target f(Variant v) { return (Target)v; }
