// N3485 8.3.5p9: an address in this unit needs no parameter layout.
struct Payload;
struct Receiver { int apply(Payload); };
int target(Payload);
int (Receiver::*selected)(Payload) = &Receiver::apply;
int (*selected_free)(Payload) = &target;
