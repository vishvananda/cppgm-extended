// VALIDATION: run-pass
// N3485 focus: 10.3 [class.virtual]

class YB { public: virtual int g() { return 1; } virtual ~YB() {} };
class YD : public YB { public: virtual int g() final { return 4; } };
int f(YB &b) { return b.g(); }
int main() { YD d; return f(d); }
