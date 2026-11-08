// expect exit 0
// P2280R4 adopted DR: passing an unknown reference does not read the referred value.
template<class T,unsigned N> constexpr unsigned extent(T (&)[N]) { return N; }
void inspect(int (&input)[3]) { static_assert(extent(input)==3,"unknown reference extent"); int local[2]={4,5}; static_assert(extent(local)==2,"known local extent"); }
int main() { int input[3]={1,2,3}; inspect(input); return 0; }
