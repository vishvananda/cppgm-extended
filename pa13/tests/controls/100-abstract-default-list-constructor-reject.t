// N3485 10.4p2, 8.5.4p3: a constructor's reference default argument also
// cannot create an abstract temporary. expect reject
struct Abstract { virtual int value() const = 0; };
struct Holder { Holder(const Abstract& = {}) {} };
