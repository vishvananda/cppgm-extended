// expect reject
// CXX11-085; N3485 11.3p6: friend definitions require a nonlocal class and unqualified name.
int main(){void target();struct Outer{struct Inner{friend void target(){}};};}
