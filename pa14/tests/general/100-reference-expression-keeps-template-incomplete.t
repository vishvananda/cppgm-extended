struct Missing;
template<class T> struct Deferred { T value; };
Deferred<Missing>& forward(Deferred<Missing>& value) { return value; }
int main() { return 0; }
