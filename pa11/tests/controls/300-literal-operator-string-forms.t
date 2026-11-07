// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// expect exit 0
using Size = decltype(sizeof(0));
int operator "" _text(const char* p, Size n) { return n != 1 || p[0] != 'x'; }
int operator "" _text(const wchar_t* p, Size n) { return n != 1 || p[0] != L'x'; }
int operator "" _text(const char16_t* p, Size n) { return n != 1 || p[0] != u'x'; }
int operator "" _text(const char32_t* p, Size n) { return n != 1 || p[0] != U'x'; }
int main() { return "x"_text || L"x"_text || u"x"_text || U"x"_text; }
