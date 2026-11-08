template<class T> void used() { throw; }
int main() { used<int>(); }
