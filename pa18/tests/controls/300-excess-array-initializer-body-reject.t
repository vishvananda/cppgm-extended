// N3485 8.5.1p6, 14.8.2p8: an invalid initializer in an instantiated
// body remains a hard error; expect reject. CXX11-013 neighbor.
template<class T> auto initialize(int) -> decltype(T{1}, int())
{
    T{1, 2};
    return 0;
}
template<class T> int initialize(...) { return 0; }
int main() { return initialize<int[1]>(0); }
