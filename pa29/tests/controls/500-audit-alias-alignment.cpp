// AUDIT-ID: PA29-ALIGN
// AUDIT-EXPECT: compile
using I __attribute__((aligned(1))) = int; I i; static_assert(__alignof__(i)==1,"alias");
typedef int J __attribute__((aligned(1))); J j; static_assert(__alignof__(j)==1,"i"); static_assert(__alignof__(*(&j))==1,"indirect"); int main(){return 0;}
