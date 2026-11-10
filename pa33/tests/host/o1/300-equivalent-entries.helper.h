#pragma once
extern volatile int tally;
struct Member {
  int value;
  Member(int n) : value(n) { tally += 1; }
  ~Member() { tally += 100; }
};
struct EntryState {
  Member member;
  int value;
  __attribute__((noinline)) EntryState(int);
  __attribute__((noinline)) ~EntryState();
};
template<int N> struct WeakState {
  int value;
  __attribute__((noinline)) WeakState(int n) : value(n) {
    for (int i = 0; i < 5; ++i) tally += n + N + i;
  }
  __attribute__((noinline)) ~WeakState() {
    for (int i = 0; i < 4; ++i) tally += value - N - i;
  }
};
int weak_work(int);
