// expect reject
// N3485 9.5p2: neither reference category can be a union variant.
union U { int&& value; };
