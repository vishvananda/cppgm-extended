// CWG 1490/1467: differently bounded rvalue references bind new mutable
// arrays; those arrays do not alias literal storage or one another. expect exit 0
// CXX11-021.
char (&&global_text)[5] = {"abc"};
const char* original = "abc\0";
int change(char (&&text)[5]) { text[0] = 'q'; return text[0] != 'q' || text[4] != 0; }

int main()
{
    char (&&local_text)[5] = {"abc"};
    global_text[0] = 'z';
    local_text[1] = 'y';
    return original[0] != 'a' || global_text[0] != 'z' || global_text[1] != 'b' ||
           local_text[0] != 'a' || local_text[1] != 'y' || local_text[4] != 0 || change({"abc"});
}
