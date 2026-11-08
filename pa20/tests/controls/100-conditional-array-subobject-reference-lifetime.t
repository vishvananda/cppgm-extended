// expect exit 0
// I13: N3485 12.2: a conditional array-subobject xvalue extends exactly the selected complete array temporary.
int live=0; struct S { S(int) { ++live; } ~S() { --live; } };
typedef S Three[3]; typedef S Four[4];
int run(bool cond) { { int i=1; S&& reference = cond ? Three{1,2,3}[i] : Four{1,2,3,4}[i]; if(live!=(cond?3:4)) return 1; } return live!=0; }
int main() { return run(true) || run(false); }
