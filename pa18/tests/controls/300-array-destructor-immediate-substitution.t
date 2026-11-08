// expect exit 0
// I13: N3485 12.2/14.8.2: array-prvalue destructor failure in an immediate context removes the candidate.
struct Bad { ~Bad() = delete; }; struct Good { ~Good() {} };
template<class T,class=decltype(T{})> char choose(int); template<class> long choose(...);
static_assert(sizeof(choose<Bad[2]>(0))==sizeof(long),"deleted array element destructor");
static_assert(sizeof(choose<Good[2]>(0))==1,"accessible array element destructor");
int main() { return 0; }
