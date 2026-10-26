#define CHOOSE(c, a, b) CHOOSE_I(c, a, b)
#define CHOOSE_I(c, a, b) CHOOSE_ ## c(a, b)
#define CHOOSE_1(a, b) a
#define CALL(x) x
CHOOSE(1, CALL, unused)(CHOOSE(1, chosen, unused))
