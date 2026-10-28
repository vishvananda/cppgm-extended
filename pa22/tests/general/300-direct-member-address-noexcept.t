struct Argument {
    Argument() noexcept {}
    Argument(const Argument&) {}
    Argument(Argument&&) noexcept {}
};
struct Object {
    int safe() const noexcept { return 7; }
    int risky() const { return 9; }
    void argument(Argument) const noexcept {}
};
template<class T> struct Conditional {
    void member() const noexcept(sizeof(T) == sizeof(int)) {}
};
Conditional<int>& nonthrowing() noexcept;
Conditional<char>& throwing() noexcept;
Object& receiver() noexcept;
Object& throwing_receiver();
Object* pointer() noexcept;
Object* throwing_pointer();
Argument& argument() noexcept;
Argument&& moved_argument() noexcept;
int (Object::*erased)() const = &Object::safe;
int (Object::*specified)() const noexcept = &Object::safe;
static_assert(noexcept((receiver().* &Object::safe)()), "direct member address");
static_assert(noexcept((receiver().* (&Object::safe))()), "parenthesized address");
static_assert(noexcept((pointer()->* &Object::safe)()), "pointer receiver");
static_assert(!noexcept((receiver().* &Object::risky)()), "throwing member");
static_assert(!noexcept((throwing_receiver().* &Object::safe)()), "throwing receiver");
static_assert(!noexcept((throwing_pointer()->* &Object::safe)()), "throwing pointer");
static_assert(!noexcept((receiver().*erased)()), "ordinary pointer loses specification");
static_assert(noexcept((receiver().*specified)()), "declared pointer specification");
static_assert(!noexcept((receiver().*static_cast<int (Object::*)() const>(&Object::safe))()),
    "cast loses specification");
static_assert(!noexcept((receiver().* &Object::argument)(argument())), "throwing argument copy");
static_assert(noexcept((receiver().* &Object::argument)(moved_argument())), "nonthrowing argument move");
static_assert(noexcept((nonthrowing().* &Conditional<int>::member)()), "deferred member specification");
static_assert(!noexcept((throwing().* &Conditional<char>::member)()), "deferred throwing specification");
int call(const Object& object) { return (object.* &Object::safe)(); }
int main() { Object object; return call(object) != 7; }
