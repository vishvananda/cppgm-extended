// N3485 14p1,14.5.4: distinguish a friend template from an ordinary friend type in a class template.
// expect exit 0
struct Owner { struct Reader; }; template<class T> class Grant { friend typename T::Reader; static int secret() { return 7; } }; struct Owner::Reader { static int read() { return Grant<Owner>::secret(); } }; int main() { return Owner::Reader::read() - 7; }
