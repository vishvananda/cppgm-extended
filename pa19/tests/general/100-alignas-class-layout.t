struct alignas(16) ForwardAligned;
struct alignas(16) ForwardAligned { char value; };
struct alignas(32) DeclAligned { char value; };
struct alignas(long double) TypeAligned { char value; };
static_assert(alignof(ForwardAligned) == 16 && sizeof(ForwardAligned) == 16, "");
static_assert(alignof(DeclAligned) == 32 && sizeof(DeclAligned) == 32, "");
static_assert(alignof(TypeAligned) == alignof(long double) && sizeof(TypeAligned) == alignof(long double), "");
int main(){return 0;}
