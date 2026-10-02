// AUDIT-ID: INHERITED-VALIDITY
// AUDIT-EXPECT: compile
struct B { B(int) {} };
struct Bad { Bad()=delete; };
struct D:B { using B::B; Bad member; };
static_assert(!__is_constructible(D,int),"deleted member");
int main(){}
