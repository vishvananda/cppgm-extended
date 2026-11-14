extern "C" {
volatile unsigned char first[] = {3, 7, 0};
alignas(16) volatile unsigned char second[] = {11, 13, 0};
volatile unsigned char zeroes[4096];
volatile unsigned char last[] = {17, 19, 0};
volatile unsigned char* tail = last;
int observe(int n) {
  zeroes[n & 4095] = 23;
  int result = first[0] + first[1] + second[0] + second[1] + tail[0] + tail[1];
  result += first[2] + second[2] + last[2] + zeroes[n & 4095];
  zeroes[n & 4095] = 0;
  return result + zeroes[(n + 1) & 4095];
}
}
