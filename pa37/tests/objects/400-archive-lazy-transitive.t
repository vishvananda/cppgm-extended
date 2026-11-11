int colliding = 13;
extern "C" int first();
extern "C" int observe() { return first() + colliding; }
