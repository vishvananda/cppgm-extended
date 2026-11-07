// N3485 14p1,14.5.4: distinguish a friend template from an ordinary friend type in a class template.
// expect exit 0
struct Reader; template<class T> class Grant { friend T; static int secret() { return 7; } }; struct Reader { static int read() { return Grant<Reader>::secret(); } }; Grant<int> ignored; int main() { return Reader::read() - 7; }
