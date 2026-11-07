// N3485 5.2.2p4: taking an address does not authorize an incomplete call.
// expect reject
struct Payload;
struct Receiver { void apply(Payload); };
void invalid(Receiver& r, Payload* p) {
    void (Receiver::*member)(Payload) = &Receiver::apply;
    (r.*member)(*p);
}
