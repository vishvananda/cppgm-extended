#define STR(x) #x
#define JOIN(a, b) a b
int first \
= 1;
int second ??/
= 2;
#define WIDE JOIN(left, \
  right)
WIDE
// comment \
continued
int third;
STR(a \
b)
