extern "C" void raise_tag(int);
extern "C" int catch_tag(int);
extern "C" int catch_reference(int);
extern "C" int catch_any(int);
extern "C" int rethrow_tag(int);
extern "C" int read_stack();
extern "C" int null_reference();
extern "C" void bridge_tag(int value) {
 volatile int live[4] = {value, 2, 3, 4};
 raise_tag(live[0]);
}
extern "C" int observe() {
 int before = read_stack();
 if (catch_tag(13) != 13 || catch_tag(7) != 7) return 1;
 if (catch_reference(13) != 14 || catch_reference(3) != 4) return 2;
 if (catch_any(9) != 17 || rethrow_tag(17) != 17) return 3;
 if (read_stack() != before) return 4;
 if (null_reference() != 1) return 5;
 return 17;
}
