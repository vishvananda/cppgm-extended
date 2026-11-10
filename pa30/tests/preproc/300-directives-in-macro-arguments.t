#define KEEP(x) x
__COUNTER__
KEEP(
start
#if __COUNTER__ == 1
selected
#else
bad
#endif
#if 0
inactive ) ,
#elif 1
elif_selected
#endif
#define VALUE replacement
VALUE
__COUNTER__
)
__COUNTER__
#define F(x) before x after
F(
#undef F
#define F(x) changed x
F(inner)
)
