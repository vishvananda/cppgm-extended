// N3485 8.5.4p3, 12.2p5: omitted runtime-initialized array elements
// remain compact and the reference keeps its mutable object. expect exit 0
int calls;
int next() { return ++calls; }
int (&&values)[16777216] = {next(), next()};
int main()
{
    if (calls != 2 || values[0] != 1 || values[1] != 2 || values[16777215] != 0)
        return 1;
    values[16777215] = 7;
    return values[16777215] != 7;
}
