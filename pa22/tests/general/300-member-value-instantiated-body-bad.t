struct Object { int function() const; };
template<class T> int call(T& object) {
    // This selected function's body is outside the immediate context.
    object.* &T::function;
    return 0;
}
int call(...) { return 1; }
int main() { Object object; return call(object); }
