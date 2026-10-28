struct Object { int function() const; };
Object& get();
// No candidate is being substituted: an uncalled member is a hard error.
using Invalid = decltype(get().* &Object::function);
