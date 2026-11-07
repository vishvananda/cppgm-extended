// N3485 14p1,14.5.4: distinguish a friend template from an ordinary friend type in a class template.
// expect reject
struct Grant { template<class T> friend T; };
