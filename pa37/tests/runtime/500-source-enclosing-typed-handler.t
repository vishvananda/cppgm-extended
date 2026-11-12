extern "C" int observe() {
 try {
  try {
   try { throw 7; }
   catch (bool) { return 1; }
  } catch (long) { return 2; }
 } catch (int value) {
  return value == 7 ? 17 : 3;
 }
 return 4;
}
