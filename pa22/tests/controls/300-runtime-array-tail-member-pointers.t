// N3485 8.5.1p7; Itanium 2.3: bulk zeroing a runtime array tail
// must retain -1 data-member nulls and zero function-member nulls. expect exit 0
struct Member { int first, second; int get() { return second; } };
struct Entry { int Member::*data; int (Member::*function)(); };
int calls;
int Member::* next() { ++calls; return &Member::second; }
Entry entries[33] = {{next(), &Member::get}};
int main()
{
    Member object = {3, 7};
    if (calls != 1 || object.*entries[0].data != 7 ||
        (object.*entries[0].function)() != 7) return 1;
    for (int i = 1; i < 33; ++i)
        if (entries[i].data != nullptr || entries[i].function != nullptr) return 2;
    return 0;
}
