// expect reject
class Base { protected: struct Node {}; };
Base::Node* make() { return nullptr; }
