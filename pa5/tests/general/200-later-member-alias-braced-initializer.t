struct token {};
int marker = 0;
struct Packet {
  int count = evaluate(marker{});
  using marker = token;
  static int evaluate(token) { return 7; }
};
int main() { Packet value; return value.count != 7; }
