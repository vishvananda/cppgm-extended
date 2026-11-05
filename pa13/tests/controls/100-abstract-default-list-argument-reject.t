// N3485 10.4p2, 8.5.4p3: {} would create an abstract temporary even when
// the parameter is a reference and this declaration is never called.
// Related upstream: GCC 15.2.0 g++.dg/cpp0x/abstract-default1.C, PR c++/83796,
// revision 5115c7e447fc07457443df874bf57840e8316d5f. expect reject
struct Abstract { virtual int value() const = 0; };
void fail(const Abstract& = {});
