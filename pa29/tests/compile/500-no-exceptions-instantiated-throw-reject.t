template<class T> void used(T value) { throw value; }
int main() { used(1); }
