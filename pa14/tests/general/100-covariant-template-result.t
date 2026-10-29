// Covariance demands each otherwise-unused return specialization's definition.
struct Value { int value; };
template<class T> struct Result : Value {
    T extra;
    void unused() { T object; object.absent(); }
};
struct Pointer { virtual Value* get() = 0; };
struct Lvalue { virtual Value& get() = 0; };
struct Rvalue { virtual Value&& get() = 0; };
template<class T> struct PointerImpl : Pointer {
    Result<T>* get() { return &value; }
    Result<T> value;
};
template<class T> struct LvalueImpl : Lvalue {
    Result<T>& get() { return value; }
    Result<T> value;
};
template<class T> struct RvalueImpl : Rvalue {
    Result<T>&& get() { return static_cast<Result<T>&&>(value); }
    Result<T> value;
};
int main() {
    PointerImpl<int> p;
    LvalueImpl<long> l;
    RvalueImpl<short> r;
    p.value.value = 17;
    l.value.value = 23;
    r.value.value = 31;
    Pointer& a = p;
    Lvalue& b = l;
    Rvalue& c = r;
    return a.get()->value + b.get().value + c.get().value - 71;
}
