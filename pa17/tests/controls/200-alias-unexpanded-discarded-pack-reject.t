// N3485 14.5.3p5: discarding an alias argument does not expand its pack.
// expect reject
template<class> using Discard = int;
template<class... Types> using Bad = Discard<Types>;
