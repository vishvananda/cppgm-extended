// N3485 14.5.3p5: decltype does not expand the pack in its expression.
// expect reject
template<class... Types> using Bad = decltype(sizeof(Types));
