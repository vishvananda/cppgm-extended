// N3485 3.6.2p2, 8.5.4p3, 12.2p5: constant array-reference temporaries
// precede dynamic initialization and retain their own identities; expect exit 0.
extern const int (&values)[4];
int read_before_definition() { return values[0] + values[1] + values[3]; }
int snapshot = read_before_definition();
const int (&values)[4] = {3, 5};
const int (&alias)[4] = {values};
const int (&other)[4] = {3, 5};
const int (&empty)[4] = {};
const int (&nested)[2][3] = {{7}, {9, 11}};
const char (&text)[4] = {"abc"};

int main()
{
    if (snapshot != 8 || &values != &alias || &values == &other)
        return 1;
    if (values[2] != 0 || empty[3] != 0 || nested[0][2] != 0 || nested[1][1] != 11)
        return 2;
    return text[2] != 'c' || text[3] != 0;
}
