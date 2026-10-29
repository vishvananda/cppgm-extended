struct Value {
    int n;
    typedef int (Value::*Pointer)() const;
    int get() const { return n; }
    operator Pointer() const { return n ? &Value::get : 0; }
};
struct Data {
    int n;
    typedef int Data::*Pointer;
    operator Pointer() const { return n ? &Data::n : 0; }
};
int calls;
Value make(int n) { ++calls; return Value{n}; }
int main() {
    Value empty{0}, full{42};
    Data zero{0}, first{42};
    if (!(empty == 0) || !(0 == empty) || empty != 0 || 0 != empty) return 1;
    if (full == 0 || 0 == full || !(full != 0) || !(0 != full)) return 2;
    if (!(empty == nullptr) || !(nullptr == empty) || empty != nullptr || nullptr != empty) return 3;
    if (full == nullptr || nullptr == full || !(full != nullptr) || !(nullptr != full)) return 4;
    if (!(zero == 0) || !(0 == zero) || zero != nullptr || nullptr != zero) return 5;
    if (first == nullptr || nullptr == first || !(first != 0) || !(0 != first)) return 6;
    if (!(empty == empty) || !(full == full) || full == empty || !(empty != full)) return 7;
    if (!(first == first) || first == zero || !(zero != first)) return 8;
    if (!(make(0) == 0) || calls != 1) return 9;
    if (!(0 != make(42)) || calls != 2) return 10;
    return 0;
}
