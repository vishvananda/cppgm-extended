struct root { int value; };
struct argument : virtual root {};
struct interface { virtual int read(argument&) = 0; };

interface& get_interface();
argument& get_argument();

int main()
{
  return get_interface().read(get_argument());
}
