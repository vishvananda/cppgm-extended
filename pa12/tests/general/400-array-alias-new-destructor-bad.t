struct Item { Item() {} private: ~Item() {} };
typedef Item Values[2];
int main() { Item* p = new Values; return p != 0; }
