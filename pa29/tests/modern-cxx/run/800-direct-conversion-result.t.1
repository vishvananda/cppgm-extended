struct Basis {};
struct Target {
    int value;
    explicit Target(int v) : value(v) {}
    Target(const Basis&) : value(100) {}
    Target(const Target&) = delete;
    Target(Target&&) = delete;
};
struct Variant {
    operator Basis() const { return Basis(); }
    operator Target() const { return Target(41); }
};
struct ConstSource;
struct Tied {
    int value;
    explicit Tied(int v) : value(v) {}
    Tied(const ConstSource&);
};
struct ConstSource { operator Tied() const { return Tied(11); } };
Tied::Tied(const ConstSource&) : value(5) {}
struct TemplateTarget {
    int value;
    explicit TemplateTarget(int v) : value(v) {}
    template<class T> TemplateTarget(const T&) : value(13) {}
};
struct TemplateSource { operator TemplateTarget() const { return TemplateTarget(17); } };
int main() {
    Variant source;
    Target a(source);
    Target b = (Target)source;
    Target c = static_cast<Target>(source);
    if (a.value != 41 || b.value != 41 || c.value != 41) return 1;
    ConstSource const_source;
    Tied e(const_source);
    if (e.value != 5) return 2;
    TemplateSource template_source;
    TemplateTarget f(template_source);
    if (f.value != 13) return 3;
}
