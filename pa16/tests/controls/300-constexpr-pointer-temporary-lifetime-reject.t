// N3485 5.19, 12.2: a pointer does not extend its temporary's lifetime;
// an address into that temporary cannot initialize a constexpr object.
// expect reject
struct Payload {
    int value;
    constexpr Payload(int n) : value(n) {}
    constexpr const int* pointer() const { return &value; }
};
constexpr const int* invalid = Payload(7).pointer();
