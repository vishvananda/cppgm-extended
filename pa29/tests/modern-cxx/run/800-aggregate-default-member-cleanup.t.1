int events, failure = 5;
struct Guard {
  int id;
  Guard(int value) : id(value) { events = events * 10 + id; if (id == failure) throw id; }
  ~Guard() { events = events * 10 + id; }
};
struct Group { int seed = 4; Guard first{seed}; Guard second{seed + 1}; };
int main() {
  try { Group group{}; return 1; }
  catch (int value) { if (value != 5 || events != 454) return 2; }
  failure = -1;
  { Group group{1}; if (group.first.id != 1 || group.second.id != 2) return 3; }
  return events != 4541221;
}
