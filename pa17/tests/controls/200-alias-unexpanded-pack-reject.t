// N3485 14.5.3p5: an alias template cannot name an unexpanded parameter pack,
// even when no specialization is requested. Related upstream case:
// GCC 15.2.0 g++.dg/cpp0x/alias-decl-17.C, PR c++/51289,
// revision 5115c7e447fc07457443df874bf57840e8316d5f. expect reject
template<class...> struct List {};
template<class... Types> using Bad = List<Types>;
