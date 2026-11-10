// N3485/N4140 7.6.1 vs C++17 P0028: a complete attribute-using-prefix requires C++17.
int f() [[using course_unknown:]]; int main(){return 0;}
