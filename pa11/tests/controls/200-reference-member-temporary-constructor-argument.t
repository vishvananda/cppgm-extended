// Adopted CWG1696/1815: temporary binding versus existing object/reference lifetime.
// expect exit 0
struct Member { int n; Member(const int& input) : n(input) {} }; struct Holder { Member member; Holder() : member(42) {} }; int main() { Holder holder; return holder.member.n-42; }
