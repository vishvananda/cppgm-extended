// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// expect exit 0
int operator "" _character(char x) { return x != 'x'; }
int operator "" _character(wchar_t x) { return x != L'x'; }
int operator "" _character(char16_t x) { return x != u'x'; }
int operator "" _character(char32_t x) { return x != U'x'; }
int main() { return 'x'_character || L'x'_character || u'x'_character || U'x'_character; }
