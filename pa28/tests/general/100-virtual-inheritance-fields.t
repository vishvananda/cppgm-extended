class YA { public: int x; };
class YB : virtual public YA {};
class YC : virtual public YA {};
class YD : public YB, public YC {};
int main() { YD d; d.x = 5; return d.x; }
