// N3485 8.5.4p3, 8.5.1p7: a runtime-valued prefix must not expand
// the omitted tail of a lifetime-extended array. expect exit 0
int calls;
unsigned char next() { return ++calls; }
const unsigned char (&values)[67108864] = {next(), next()};
int main()
{
    return calls != 2 || values[0] != 1 || values[1] != 2 ||
           values[2] != 0 || values[67108863] != 0;
}
