// N3485 8.3.5p9 and 5.2.2p4: declarations and addresses do not require
// parameter layout; calls and definitions do.
struct Arg; void target(Arg); void (*global)(Arg)=&target; void capture() { void (*local)(Arg)=&target; (void)local; }
