#include "500-virtual-base-runtime.h"
extern "C" { int destruction; }
extern "C" int observe() {
 destruction = 0;
 {
  Whole object;
  Shared* shared = &object;
  Left* left = &object;
  Right* right = &object;
  if (static_cast<Shared*>(left) != static_cast<Shared*>(right)) return 1;
  if (object.Left::during != 14 || object.Right::during != 16) return 2;
  if (visit_shared(shared) != 26 || self_shared(shared) != shared) return 3;
  if (cross_shared(left) != right || dynamic_cast<Whole*>(shared) != &object) return 4;
  if (dynamic_cast<void*>(right) != static_cast<void*>(&object)) return 5;
  if (typeid(*shared) != typeid(Whole) || typeid(*right) != typeid(Whole)) return 6;
 }
 if (destruction != 26) return 7;
 Shared* allocated = make_shared();
 if (visit_shared(allocated) != 26 || self_shared(allocated) != allocated) return 8;
 Whole* whole = dynamic_cast<Whole*>(allocated);
 if (!whole || whole->Left::during != 14 || whole->Right::during != 16) return 9;
 delete_shared(allocated);
 if (destruction != 52) return 10;
 Shared* last = make_shared();
 Whole* complete = dynamic_cast<Whole*>(last);
 if (finish_shared(last) != static_cast<void*>(complete)) return 11;
 ::operator delete(complete);
 if (destruction != 78) return 12;
 return 17;
}
