// expect exit 0
// I13: Adopted CWG2894/2941: a braced functional reference cast retains the temporary through the receiving reference lifetime.
int live=0, dead=0; struct S { S() { ++live; } ~S() { --live; ++dead; } };
typedef const S& Reference;
int main() { { const S& value=Reference{}; if(live!=1 || dead!=0) return 1; } return live!=0 || dead!=1; }
