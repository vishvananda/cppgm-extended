// VALIDATION: run-pass
// N3485 focus: core-language reduction of map subscript insertion surface

struct value_box
{
  int value;

  value_box() : value(0) {}
};

template<typename Key, typename Value>
struct entry
{
  Key key;
  Value value;

  explicit entry(const Key & init_key) : key(init_key), value() {}
};

template<typename Key, typename Value>
struct tiny_assoc
{
  entry<Key, Value> * slot;

  tiny_assoc() : slot(0) {}

  ~tiny_assoc()
  {
    delete slot;
  }

  Value & subscript(const Key & key)
  {
    if(!slot || slot->key != key) {
      delete slot;
      slot = new entry<Key, Value>(key);
    }
    return slot->value;
  }
};

int main()
{
  tiny_assoc<unsigned char, value_box> values;
  unsigned char key = 1;
  values.subscript(key).value = 3;
  return values.subscript(key).value == 3 ? 0 : 1;
}
