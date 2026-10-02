// AUDIT-ID: CONST-BITFIELD
// AUDIT-EXPECT: compile
struct S { unsigned x:2; constexpr explicit operator bool()const{return x==1;} }; template<class T> int f(){static_assert(S{5},"truncate field");return 0;} int main(){return f<int>();}
