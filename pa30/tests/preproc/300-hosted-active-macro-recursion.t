// Hosted GNU rescanning: a deferred tail takes its parentheses from a still-open
// enclosing expansion. Its replacement must retain that expansion's suppression.
#define EMPTY()
#define DEFER(m) m EMPTY()
#define BRIDGE() CALL
#define CALL() ID(9)
#define ID(x) x
ID(DEFER(BRIDGE)()())

// Once unavailable, that name remains unavailable through another rescan.
#define RESCAN(x) x
RESCAN(ID(DEFER(BRIDGE)()()))

// An independent invocation and a selected name whose enclosing expansion has
// ended expand normally; suppression is local to the earlier token.
BRIDGE()()
#define SELECT(f) f
SELECT(BRIDGE)()()

// More than one ending helper context may bridge into the live expansion.
#define BRIDGE_AGAIN() BRIDGE
ID(DEFER(BRIDGE_AGAIN)()()())

// Copies of an already unavailable name retain its paint.
#define TWICE(a) a a
TWICE(ID(DEFER(BRIDGE)()()))

// A live normal ancestor, as well as the immediate containing macro, remains
// suppressed when a selected argument starts a nested expansion.
#define AN_OUTER(x) x
#define AN_INNER(x) x
#define AN_BRIDGE() AN_ACTIVE(9)
#define AN_ACTIVE(x) AN_INNER(DEFER(AN_BRIDGE)())
AN_OUTER(DEFER(AN_ACTIVE)(0))

// The selected name may expand directly instead of through another tail.
#define DIRECT() ID(9)
ID(DEFER(DIRECT)())

// Hosted recursion follows GNU replacement contexts, including calls that take
// their parentheses from the source after an earlier context has ended.
#define HOST_F(x) 1 x
#define HOST_G(x) 2 x
HOST_G(HOST_F)(HOST_G)(3)

// Disablement is checked while the final token's context is still open,
// before looking past that context for a following parenthesis.
#define HOST_LAST(x) tag x
HOST_LAST(HOST_LAST)(2)

// A directive encountered during argument collection has its own hosted
// rescan cursor; it follows the same recursion rules as ordinary text.
#define COND_F(x) +1 x
#define COND_G(x) +2 x
#define HOST_WRAP(x) x
HOST_WRAP(
#if 0 COND_G(COND_F)(COND_G)(+3) == 8
77
#else
#error hosted macro recursion must also apply to directive arguments
#endif
)

// Capturing an old definition does not let its replacement recursively enter
// a new definition of the same name. The new definition works after it ends.
#define HOST_SELF(x) HOST_SELF x
HOST_SELF(
#undef HOST_SELF
#define HOST_SELF(x) changed x
(7))
HOST_SELF(8)
