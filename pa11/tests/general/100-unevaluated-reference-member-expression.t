struct Context { int get()const; };
struct Holder { Context &context; };
int main() { decltype(Holder::context.get()) value=7; return value==7?0:1; }
