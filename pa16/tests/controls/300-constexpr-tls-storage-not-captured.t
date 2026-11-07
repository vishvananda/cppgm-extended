// expect exit 0
// Nonautomatic constexpr storage is odr-usable from a local class; no capture is needed.
int main(){thread_local constexpr int n=7;struct Local{const int* pointer(){return &n;}const int& ref(){return n;}};Local a;return a.pointer()!=&n || &a.ref()!=&n;}
