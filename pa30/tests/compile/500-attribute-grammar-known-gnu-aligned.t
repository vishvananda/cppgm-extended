// N3485 7.6.1: attribute-token, optional parenthesized balanced arguments and comma separators.
struct [[gnu::aligned(32)]] X{char c;};
static_assert(alignof(X)==32,"explicit alignment retained");
int main(){return alignof(X)==32?0:1;}
