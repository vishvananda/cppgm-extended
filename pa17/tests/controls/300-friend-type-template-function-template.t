// N3485 14p1,14.5.4: distinguish a friend template from an ordinary friend type in a class template.
// expect exit 0
class Grant { static int secret() { return 7; } template<class T> friend int read(Grant, T) { return secret(); } }; int main() { Grant grant; return read(grant, 3) + read(grant, 'a') - 14; }
