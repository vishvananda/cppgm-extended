struct Nested { unsigned char tag; const char* pointer; unsigned char last; };
struct Row { const char* name; Nested value; };
const Row rows[] = {
 {"first", {3, nullptr, 5}}, {"second", {7, "present", 9}},
 {"third", {11, nullptr, 13}}, {"fourth", {17, nullptr, 19}}
};
extern "C" int observe() {
 for (unsigned i = 0; i < 4; ++i) {
  if (!rows[i].name || rows[i].value.tag != 3 + i * 4 + (i == 3 ? 2 : 0)) return 1;
  if (rows[i].value.last != rows[i].value.tag + 2) return 2;
  if ((rows[i].value.pointer != nullptr) != (i == 1)) return 3;
 }
 return rows[3].name[0] == 'f' && rows[1].value.pointer[0] == 'p' ? 17 : 4;
}
