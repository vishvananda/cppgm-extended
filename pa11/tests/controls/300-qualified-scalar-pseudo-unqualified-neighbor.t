// expect exit 0
// N3485 3.4.3p6, 5.2.4p1-p2: qualified scalar pseudo-destructor lookup and type identity.
typedef int I; int evaluations; int* choose(int* p){++evaluations;return p;} int main(){int v=7; choose(&v)->~I(); return evaluations==1?0:1;}
