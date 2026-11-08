// expect exit 0
// I13: Adopted CWG2894: reference functional casts have glvalue category and initialize their referred value.
typedef const int& L; typedef int&& R; int main() { const int& a=L{}; int&& b=R{}; b=7; return a!=0 || b!=7; }
