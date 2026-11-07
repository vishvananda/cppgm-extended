// N3485 14p1,14.5.4: distinguish a friend template from an ordinary friend type in a class template.
// expect exit 0
struct Reader; typedef Reader Alias; class Grant { friend Alias; static int secret() { return 7; } }; struct Reader { static int read() { return Grant::secret(); } }; int main() { return Reader::read() - 7; }
