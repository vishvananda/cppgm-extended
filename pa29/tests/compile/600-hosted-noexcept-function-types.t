template<class A, class B> struct Same { static constexpr bool value = false; };
template<class A> struct Same<A, A> { static constexpr bool value = true; };

using Throwing = int(*)();
using Nonthrowing = int(*)() noexcept;
static_assert(!Same<Throwing, Nonthrowing>::value, "exception specification is part of the type");

template<class> struct Signature;
template<class R, class C, class... A>
struct Signature<R(C::*)(A...) const &> { static constexpr int value = 1; };
template<class R, class C, class... A>
struct Signature<R(C::*)(A...) const & noexcept> { static constexpr int value = 2; };

struct Example { int normal() const &; int safe() const & noexcept; };
static_assert(Signature<decltype(&Example::normal)>::value == 1, "throwing member type");
static_assert(Signature<decltype(&Example::safe)>::value == 2, "nonthrowing member type");

template<bool B> using Conditional = int(*)() noexcept(B);
static_assert(Same<Conditional<true>, Nonthrowing>::value, "substituted true specification");
static_assert(Same<Conditional<false>, Throwing>::value, "false specification is potentially throwing");

int safe() noexcept;
int normal();
Throwing erased = &safe;
Nonthrowing retained = &safe;
int (&function_reference)() = safe;

constexpr int select(Throwing) { return 1; }
constexpr int select(Nonthrowing) { return 2; }
static_assert(select(&safe) == 2, "identity beats function-pointer conversion");
static_assert(noexcept(retained()), "indirect call follows the function type");
static_assert(!noexcept(erased()), "conversion erased the guarantee");

struct CompleteClass {
  int later() noexcept(property);
  static constexpr bool property = true;
};
using Expected = int(CompleteClass::*)() noexcept;
static_assert(Same<decltype(&CompleteClass::later), Expected>::value, "complete-class context");

static_assert(Same<decltype(true ? &safe : &normal), Throwing>::value,
              "conditional expression discards the guarantee");
bool compare(Throwing left) { return left == safe && safe != left; }
using Member = int(Example::*)() const &;
using SafeMember = int(Example::*)() const & noexcept;
Member member_conditional = true ? Member(&Example::normal) : SafeMember(&Example::safe);
bool compare_members(Member left, SafeMember right) { return left == right && right != left; }

 template<bool B> constexpr bool deduce(int(*)() noexcept(B)) { return B; }
int safe() noexcept;
int normal();
static_assert(deduce(&safe), "deduce true");
static_assert(!deduce(&normal), "deduce false");
template<bool B> struct Tag {};
template<bool B> Tag<B> signature(int(*)() noexcept(B));
static_assert(Same<decltype(signature(&safe)),Tag<true>>::value, "noexcept argument deduction");
template<class T> void instantiate() noexcept(true) {}
template void instantiate<int>();
