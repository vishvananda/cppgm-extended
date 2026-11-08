struct Object {
  Object() {}
  Object(const Object&) = delete;
  Object(Object&&) = delete;
};
Object make() { return Object(); }
