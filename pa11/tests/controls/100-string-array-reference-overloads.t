// CWG 1467: a string-to-character-array list conversion preserves literal
// kinds; ordinary scalar lists retain their ranking. expect exit 0
// CXX11-021: failed candidates must not hide a valid overload.
int kind(const char16_t (&)[5]) { return 1; }
int kind(const char (&)[5]) { return 0; }
int scalar(const char (&)[1]) { return 0; }
int scalar(const int (&)[1]) { return 1; }

int main()
{
    return kind({"abc"}) || scalar({'a'});
}
