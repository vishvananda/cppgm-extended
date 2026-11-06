// N3485 8.5.1p7, 8.5.4p3: zero aggregate tails need no dense constant
// storage when an ordinary reference's initialization is probed. expect exit 0
struct Element { int value; };
int main()
{
    const Element (&values)[1048576] = {};
    return values[0].value != 0 || values[1048575].value != 0;
}
