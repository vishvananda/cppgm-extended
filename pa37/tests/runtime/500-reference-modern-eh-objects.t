extern "C" int reference_case(int);
extern "C" int observe() {
 if (reference_case(0) != 100) return 1;
 if (reference_case(2) != 103) return 2;
 if (reference_case(4) != 117) return 3;
 if (reference_case(2) != 103) return 4;
 return 17;
}
