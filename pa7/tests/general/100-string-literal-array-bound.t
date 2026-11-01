// 8.5.2p2: an array of unknown bound holds the literal's code units in its
// element type's encoding, and its terminating null.
template<bool B> struct check;
template<> struct check<true> { typedef int ok; };

char plain[] = "é";
char braced[] = { "é" };
char universal[] = "é";
char astral[] = "😀";
char joined[] = "a" "é";
char raw[] = R"(é)";
char utf8[] = u8"é";
char escaped[] = "\xff\n";
char16_t bmp16[] = u"é";
char16_t astral16[] = u"😀";
char16_t raw16[] = uR"(😀)";
char32_t astral32[] = U"😀";
wchar_t wide[] = L"é";

check<sizeof(plain) == 3>::ok a;
check<sizeof(braced) == 3>::ok b;
check<sizeof(universal) == 3>::ok c;
check<sizeof(astral) == 5>::ok d;
check<sizeof(joined) == 4>::ok e;
check<sizeof(raw) == 3>::ok f;
check<sizeof(utf8) == 3>::ok g;
check<sizeof(escaped) == 3>::ok h;
check<sizeof(bmp16) / sizeof(char16_t) == 2>::ok i;
check<sizeof(astral16) / sizeof(char16_t) == 3>::ok j;
check<sizeof(raw16) / sizeof(char16_t) == 3>::ok k;
check<sizeof(astral32) / sizeof(char32_t) == 2>::ok l;
check<sizeof(wide) / sizeof(wchar_t) == 2>::ok m;
