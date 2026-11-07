// N3485 8.3.5p9 and 5.2.2p4: declarations and addresses do not require
// parameter layout; calls and definitions do.
struct Arg; struct C { void target(Arg); }; void capture() { void (C::*local)(Arg)=&C::target; (void)local; }
