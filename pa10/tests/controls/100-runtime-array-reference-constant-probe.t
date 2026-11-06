// N3485 8.5.4p3, 8.5.1p7: a runtime prefix is valid; optional constant
// evaluation must not allocate every implicit element. expect exit 0
int calls;
int next() { return ++calls; }
int main()
{
    const int (&values)[1048576] = {next()};
    return calls != 1 || values[0] != 1 || values[1048575] != 0;
}
