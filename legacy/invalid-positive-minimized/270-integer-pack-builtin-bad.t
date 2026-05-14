// Minimized from pa22/tests/general/270-integer-pack-tuple-defer.t
template<int... I>
struct seq {};

using X = seq<__integer_pack(3)...>;
