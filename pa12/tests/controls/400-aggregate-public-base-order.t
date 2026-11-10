struct Base { int first; };
struct Value:Base { int second=7; };
int main() { Value v{{3}}; return v.first==3 && v.second==7?0:1; }
