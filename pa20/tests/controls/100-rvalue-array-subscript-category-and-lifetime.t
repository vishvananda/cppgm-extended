// expect exit 0
// I13: Adopted CWG1213/1299: either array operand order preserves an xvalue subobject and complete-array lifetime.
int live=0; struct S { int n; S(int v):n(v) { ++live; } ~S() { --live; } }; typedef S Array[3];
int main() { { int i=1; S&& first=Array{1,2,3}[i]; if(live!=3 || first.n!=2) return 1; S&& commuted=i[Array{4,5,6}]; if(live!=6 || commuted.n!=5) return 2; first.n=9; if(first.n!=9 || commuted.n!=5) return 3; } return live!=0; }
