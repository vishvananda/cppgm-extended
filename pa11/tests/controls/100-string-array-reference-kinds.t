// CWG 1490/1467 and 8.5.2: preserve literal kind, narrow code units and
// implicit zero padding in reference-target arrays. expect exit 0
// CXX11-021.
int main()
{
    const signed char (&signed_text)[5] = {"a"};
    const unsigned char (&unsigned_text)[5] = {"\xff"};
    const wchar_t (&wide_text)[5] = {L"\u03bb"};
    const char16_t (&utf16_text)[5] = {u"\u03bb"};
    const char32_t (&utf32_text)[5] = {U"\U0001f642"};
    if (signed_text[0] != 'a' || signed_text[1] != 0 || signed_text[4] != 0)
        return 1;
    if (unsigned_text[0] != 255 || unsigned_text[1] != 0 || unsigned_text[4] != 0)
        return 2;
    if (wide_text[0] != L'\u03bb' || wide_text[1] != 0 || wide_text[4] != 0)
        return 3;
    if (utf16_text[0] != u'\u03bb' || utf16_text[1] != 0 || utf16_text[4] != 0)
        return 4;
    return utf32_text[0] != U'\U0001f642' || utf32_text[1] != 0 || utf32_text[4] != 0;
}
