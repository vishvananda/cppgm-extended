// N3485 14.5.3p5: an ordinary member alias also cannot name a bare pack.
// expect reject
template<class... Types> struct Bad { using Type = Types*; };
