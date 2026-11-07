// expect exit 0
// CWG2346/P0588R1: static storage is not an automatic local entity, even when constexpr.
const int* pointer(const int* p){return p;}const int& ref(const int& p){return p;}
int main(){static constexpr int n=9;const int* pointer(const int* = &n);const int& ref(const int& = n);return pointer()!=&n || &ref()!=&n;}
