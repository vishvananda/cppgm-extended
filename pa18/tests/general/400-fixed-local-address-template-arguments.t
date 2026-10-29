int object = 7;
struct owner { static int member; };
int owner::member = 11;
int read_value() { return 19; }

template<int* Pointer>
struct pointer_holder
{
  typedef int type;
  static int read() { return *Pointer; }
};

template<int& Reference>
struct reference_holder
{
  typedef int type;
  static int read() { return Reference; }
};

template<int (*Function)()>
struct function_holder
{
  typedef int type;
  static int read() { return Function(); }
};

template<class Unused>
int check_addresses()
{
  typedef pointer_holder<&object> pointer;
  typedef pointer_holder<&owner::member> member;
  typedef reference_holder<object> reference;
  typedef function_holder<&read_value> function;
  pointer::type a = pointer::read();
  member::type b = member::read();
  reference::type c = reference::read();
  function::type d = function::read();
  return a == 7 && b == 11 && c == 7 && d == 19 ? 0 : 1;
}

template<class T> struct private_reader;
class private_owner
{
  static int value;
  friend struct private_reader<int>;
};
int private_owner::value = 23;

template<class T>
struct private_reader
{
  static int read()
  {
    typedef pointer_holder<&private_owner::value> argument;
    typename argument::type value = argument::read();
    return value == 23 ? 0 : 1;
  }
};

int main()
{
  return check_addresses<void>() + check_addresses<long>() + private_reader<int>::read();
}
