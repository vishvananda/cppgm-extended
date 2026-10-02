// AUDIT-ID: CONST-REF-STATIC-TEMP
// AUDIT-EXPECT: run
constexpr const int& ref=3;static_assert(ref==3,"constant");int main(){return ref!=3;}
