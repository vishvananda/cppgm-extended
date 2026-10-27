// A valid substituted signature does not suppress a function-body error.
struct Object { int value; typedef int setup; };
template<class T> auto call(T& object) -> decltype(object.value, int()) {
    object.setup();
    return 0;
}
int main() { Object object; return call(object); }
