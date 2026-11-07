// Adopted CWG727 supersedes the namespace-only restriction of N3485 14.7.3p2.
// The specialization is valid where its primary member template may be defined; expect exit 0.
struct S { template<class T> int f(T) { return 1; } template<> int f<int>(int) { return 2; } };
int main() { S object; return object.f<int>(0) != 2 || object.f<char>(0) != 1; }
