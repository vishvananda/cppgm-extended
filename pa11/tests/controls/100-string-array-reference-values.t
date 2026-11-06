// CWG 1490/1467 (2014): a string initializes the differently bounded
// character-array temporary of a list-initialized reference. expect exit 0
// CXX11-021: independently reduced declaration and lifetime coverage.
const char (&global_text)[5] = {"abc"};

int main()
{
    const char (&local_text)[5]{"xyz"};
    const char (&copy_text)[5] = {"abc"};
    const char* volatile global_address = &global_text[0];
    const char* volatile copy_address = &copy_text[0];
    if (global_text[0] != 'a' || global_text[2] != 'c' || global_text[3] != 0 || global_text[4] != 0)
        return 1;
    if (global_address == copy_address)
        return 2;
    return local_text[0] != 'x' || local_text[2] != 'z' || local_text[3] != 0 || local_text[4] != 0;
}
