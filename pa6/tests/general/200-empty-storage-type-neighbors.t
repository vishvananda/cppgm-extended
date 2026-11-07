// N3485 7p3 permits an omitted init-declarator-list for these type declarations.
struct Forward;
struct Forward { int member; };
class Other;
enum Value { one = 1, two = 2 };
