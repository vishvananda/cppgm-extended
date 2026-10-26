// N3485 16.3: replacement tokens keep invocation locations and recursion
// state through argument substitution, stringizing, pasting and empty inputs.
#define TEXT_(x) #x
#define TEXT(x) TEXT_(x)
#define JOIN_(a,b) a##b
#define JOIN(a,b) JOIN_(a,b)
#define BODY(x) head x tail __LINE__
#define REENTER(x) BODY(x) REENTER
#line 200
TEXT(BODY(JOIN(1,2)))
BODY()
REENTER(BODY(3))(4)
#define FIRST_EMPTY(a,b) a b
FIRST_EMPTY(,BODY(5))
#define VARIADIC(...) prefix __VA_ARGS__ suffix
VARIADIC(BODY(6))
