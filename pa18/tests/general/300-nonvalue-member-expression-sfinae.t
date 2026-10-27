// N3485 14.8.2: invalid expressions in a signature discard that candidate.
template<class T> T&& declval();
template<class> struct Accept { typedef char type; };
template<class T> struct HasSetup {
    template<class U>
    static auto check(U*) -> typename Accept<decltype(declval<U>().setup())>::type;
    template<class U> static int check(...);
    static const bool value = sizeof(check<T>(0)) == sizeof(char);
};
template<class T> auto member_value(int) -> decltype(declval<T>().setup, char());
template<class T> int member_value(...);
struct Missing {};
struct setup {};
struct Derived : setup {};
struct Alias { typedef int setup; };
struct Nested { struct setup {}; };
struct Enumeration { enum setup { entry }; };
struct ClassTemplate { template<class> struct setup; };
struct AliasTemplate { template<class T> using setup = T; };
struct Data { int setup; };
struct Function { void setup(); };
struct StaticFunction { static void setup(); };
static_assert(!HasSetup<Missing>::value, "missing name");
static_assert(!HasSetup<setup>::value, "injected class name");
static_assert(!HasSetup<Derived>::value, "inherited injected class name");
static_assert(!HasSetup<Alias>::value, "member typedef");
static_assert(!HasSetup<Nested>::value, "nested class");
static_assert(!HasSetup<Enumeration>::value, "member enum type");
static_assert(!HasSetup<ClassTemplate>::value, "member class template");
static_assert(!HasSetup<AliasTemplate>::value, "member alias template");
static_assert(!HasSetup<Data>::value, "noncallable data");
static_assert(HasSetup<Function>::value, "member function");
static_assert(HasSetup<StaticFunction>::value, "static member function");
static_assert(sizeof(member_value<Alias>(0)) == sizeof(int), "type is not an expression");
static_assert(sizeof(member_value<Data>(0)) == sizeof(char), "data is an expression");
int main() { return 0; }
