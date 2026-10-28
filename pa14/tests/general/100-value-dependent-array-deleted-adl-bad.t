// A better associated overload declared after the template is visible to
// a dependent operator call, including when that winning overload is deleted.
struct Stream {
  Stream& operator<<(const void*) { return *this; }
};
template<class T> void write(Stream& out) {
  char text[sizeof(T)] = {};
  out << text;
}
Stream& operator<<(Stream&, const char*) = delete;
int main() { Stream out; write<int>(out); }
