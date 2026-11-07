// N3485 8.3.5p9: passing a function address does not call that function.
struct Payload;
void target(Payload);
void consume(void (*function)(Payload));
void pass() { consume(&target); }
