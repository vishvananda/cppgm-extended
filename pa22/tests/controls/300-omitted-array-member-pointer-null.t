// N3485 8.5p5, 8.5.1p7; Itanium ABI 2.3: a null data-member pointer
// is -1, even inside an omitted array tail; expect exit 0.
// CXX11-007 neighbor: zero-byte compression must retain ABI null values.
struct Target { int member; };
int Target::*members[4] = {&Target::member};
int Target::*omitted[4] = {};
struct Row { int Target::*pointers[2]; int padding; };
Row rows[3] = {{{&Target::member}}};
Row omitted_rows[3] = {};

int main()
{
    Target object = {9};
    if (object.*members[0] != 9 || members[1] != nullptr || members[3] != nullptr)
        return 1;
    if (rows[0].pointers[0] != &Target::member || rows[0].pointers[1] != nullptr ||
        rows[1].pointers[0] != nullptr || rows[2].pointers[1] != nullptr || rows[2].padding != 0)
        return 2;
    if (omitted[0] != nullptr || omitted[3] != nullptr ||
        omitted_rows[0].pointers[0] != nullptr || omitted_rows[2].pointers[1] != nullptr ||
        omitted_rows[2].padding != 0)
        return 3;
    return 0;
}
