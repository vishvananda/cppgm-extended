// VALIDATION: run-pass
// N3485 focus: 10.3 [class.virtual]

class YB { public: virtual int f() { return 1; } virtual ~YB() {} };
class YD : public YB { public: int f() override { return 3; } };
int g(YB *b) { return b->f(); }
int main() { YD d; return g(&d); }
