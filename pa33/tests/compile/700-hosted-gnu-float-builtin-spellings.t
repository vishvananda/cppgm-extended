void f(long double);
void f(__float128);

void g(double);
void g(_Float64);

static_assert(__is_floating_point(__float128), "__float128 is floating");
static_assert(__is_floating_point(_Float64), "_Float64 is floating");

int main() { return 0; }
