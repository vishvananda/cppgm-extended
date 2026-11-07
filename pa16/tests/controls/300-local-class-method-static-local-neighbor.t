// expect exit 0
// N3485 6.7p4 versus9.8p4: a function-local static is not a static data member.
int main() { struct Local { static int next() { static int counter = 3; return ++counter; } }; return Local::next() != 4 || Local::next() != 5; }
