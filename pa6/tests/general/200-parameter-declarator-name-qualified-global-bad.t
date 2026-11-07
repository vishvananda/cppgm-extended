// N3485 8.3p1,3.3.4: a parameter declarator names an ordinary unqualified identifier.
int n; void f(int ::n); int main(){return 0;}
