struct Meta{static int value;};
Meta make_meta();
using Unknown=decltype(make_meta());
struct Call{Unknown::value operator()();};
