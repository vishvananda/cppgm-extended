// N3485 5.19, 12.2, 12.6: each caller-owned class keeps its initialized array across constructor returns. expect exit 0
struct E { int values[4]; constexpr E(int n):values{n,n+1,n+2,n+3}{} };
constexpr int read(const E& a,const E& b) {return a.values[0]+b.values[3];}
static_assert(read({1},{7})==11, "separate array payloads");
int main(){return read({1},{7})!=11;}
