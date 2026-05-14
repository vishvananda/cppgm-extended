// VALIDATION: run-pass
// N3485 focus: 10.3 [class.virtual]

class YB { public: virtual int f() { return 7; } virtual ~YB() {} };
class YD : public YB { };
int g(YB &b) { return b.f(); }
int main() { YD d; return g(d); }
