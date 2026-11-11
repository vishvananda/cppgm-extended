extern "C" int* first_address();
extern "C" int* second_address();
extern "C" int observe() {
 int* a = first_address();
 int* b = second_address();
 *a += 2;
 *b += 3;
 return a == b ? *a : 0;
}
