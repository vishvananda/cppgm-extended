// Sequential by-value copies can reuse storage; a call's arguments must coexist.
struct Payload { long long words[8]; };
extern "C" __attribute__((noinline)) long long consume(Payload value) {
  value.words[0] += 1;
  volatile long long* observed = value.words;
  return *observed;
}
extern "C" __attribute__((noinline)) long long together(Payload first, Payload second) {
  volatile long long* a = first.words;
  volatile long long* b = second.words;
  return *a * 100 + *b;
}
extern "C" long long simultaneous() {
  Payload first = {{7}}, second = {{11}};
  return together(first, second);
}
extern "C" __attribute__((noinline)) long long recursive_copies(int depth) {
  Payload value = {{depth}};
  long long result = 0;
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  result += consume(value);
  return result + (depth ? recursive_copies(depth - 1) : 0);
}
