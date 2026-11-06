// C++17 dcl.attr.grammar p2: a using-prefix has the same GNU attribute effect as an explicit namespace.
struct [[using gnu: packed]] X{char c;int i;};
static_assert(sizeof(X)==5,"using-prefix preserves packed layout");
int main(){return sizeof(X)==5?0:1;}
