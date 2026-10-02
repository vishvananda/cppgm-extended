// AUDIT-ID: REF-BITFIELD
// AUDIT-EXPECT: run
struct S{unsigned x:2;};int main(){S s;s.x=1;const unsigned& r=s.x;s.x=2;return r!=1;}
