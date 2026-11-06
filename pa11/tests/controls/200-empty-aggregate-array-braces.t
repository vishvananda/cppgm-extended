// N3485 8.5.1p4,7,8: explicit empty aggregate clauses infer the bound;
// expect exit 0. Omitted trailing members are still allowed.
struct Empty {};
Empty elements[] = {{}, {}, {}};
struct Row { Empty first; int value; Empty last; };
Row rows[2] = {{{}, 7}};

int main()
{
    return sizeof(elements) != 3 * sizeof(Empty) || rows[0].value != 7 || rows[1].value != 0;
}
