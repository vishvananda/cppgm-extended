// expect exit 0
// N3485 12.2p5 / CWG1299: a glvalue conditional extends the converting cast temporary in its selected branch.
int live=0,dead=0; struct S { int value; S(int n):value(n) { ++live; } ~S() { --live; ++dead; } };
int run(bool first,bool second) { int before=dead; { S&& value=first ? second ? static_cast<S&&>(1) : static_cast<S&&>(2) : static_cast<S&&>(3); if(live!=1 || dead!=before || value.value!=(first?second?1:2:3)) return 1; } return live!=0 || dead!=before+1; }
int main() { return run(true,true) || run(true,false) || run(false,true) || run(false,false); }
