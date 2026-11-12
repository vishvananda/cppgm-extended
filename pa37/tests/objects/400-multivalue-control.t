extern "C" int evaluate(int);
extern "C" int count_down(int);
extern "C" int choose_tuple(int);
extern "C" int identity_if(int, int);
extern "C" int table_exit(int);
extern "C" int polymorphic_dead();
extern "C" int wide_literal();
extern "C" int observe() {
 if (evaluate(8) != 17 || evaluate(3) != 12) return 1;
 if (count_down(5) != 15 || count_down(1) != 1) return 2;
 if (choose_tuple(0) != 17 || choose_tuple(1) != 11) return 3;
 if (identity_if(16, 0) != 16 || identity_if(16, 1) != 17) return 4;
 if (table_exit(0) != 18 || table_exit(1) != 17 || table_exit(99) != 17) return 5;
 if (polymorphic_dead() != 17 || wide_literal() != 1) return 6;
 return 17;
}
