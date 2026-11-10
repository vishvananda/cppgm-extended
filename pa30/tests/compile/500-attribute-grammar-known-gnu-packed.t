// N3485 7.6.1: attribute-token, optional parenthesized balanced arguments and comma separators.
struct [[gnu::packed]] X{char c;int i;};
static_assert(sizeof(X)==5,"packed layout retained");
int main(){return sizeof(X)==5?0:1;}
