// N3485/N4140 7.6.1 vs C++17 P0028: a complete attribute-using-prefix requires C++17.
using P=int * [[using course_unknown:]]; P p=0; int main(){return p?1:0;}
