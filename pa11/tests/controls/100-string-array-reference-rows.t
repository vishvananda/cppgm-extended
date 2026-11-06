// CWG 1490/1467: nested array lists keep each string's own bound and
// padding while the enclosing array binds as a temporary. expect exit 0
// CXX11-021: already-supported nested-list neighbor.
int main()
{
    const char (&rows)[2][5] = {{"abc"}, {"xy"}};
    return rows[0][0] != 'a' || rows[0][3] != 0 || rows[0][4] != 0 ||
           rows[1][0] != 'x' || rows[1][1] != 'y' || rows[1][2] != 0 || rows[1][4] != 0;
}
