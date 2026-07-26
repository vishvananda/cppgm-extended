int destroyed;

struct object
{
  ~object() { ++destroyed; }
};

object make() { return object(); }
bool operator==(const object&, const object&) { return true; }

int main()
{
  object value;
  if(value == make() || false) {}
  return destroyed == 1 ? 0 : 1;
}
