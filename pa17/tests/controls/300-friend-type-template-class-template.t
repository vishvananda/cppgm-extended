// N3485 14p1,14.5.4: distinguish a friend template from an ordinary friend type in a class template.
// expect exit 0
class Grant { template<class T> friend struct Reader; static int secret() { return 7; } }; template<class T> struct Reader { static int read() { return Grant::secret(); } }; template<class T> struct Reader<T*> { static int read() { return Grant::secret() + 1; } }; int main() { return Reader<int>::read() + Reader<int*>::read() - 15; }
