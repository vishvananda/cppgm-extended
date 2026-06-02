template<class T>
struct switch_condition_body {
  int run() {
    switch(int tag = 0) {
    case 0:
      return tag;
    default:
      return 1;
    }
  }
};

int main() {
  return 0;
}
