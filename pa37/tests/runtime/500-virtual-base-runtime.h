#include <typeinfo>
#include <new>
extern "C" int destruction;
struct Shared {
 int value;
 Shared(int n) : value(n) {}
 virtual ~Shared() { destruction += value; }
 virtual int score() { return value; }
 virtual Shared* self() { return this; }
};
extern "C" int visit_shared(Shared*);
struct Left : virtual Shared {
 int left, during;
 Left() : Shared(100), left(3), during(visit_shared(this)) {}
 ~Left() { destruction += left; }
 int score() override { return value + left; }
};
struct Right : virtual Shared {
 int right, during;
 Right() : Shared(200), right(5), during(visit_shared(this)) {}
 ~Right() { destruction += right; }
 int score() override { return value + right; }
};
struct Whole : Left, Right {
 int own;
 Whole() : Shared(11), Left(), Right(), own(7) {}
 ~Whole() { destruction += own; }
 int score() override { return value + left + right + own; }
 Whole* self() override { return this; }
};
extern "C" Shared* make_shared();
extern "C" Shared* self_shared(Shared*);
extern "C" Right* cross_shared(Left*);
extern "C" void delete_shared(Shared*);
extern "C" void* finish_shared(Shared*);
