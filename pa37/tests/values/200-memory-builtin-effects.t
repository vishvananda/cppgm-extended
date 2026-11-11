extern "C" int observe() {
 unsigned char data[12] = {};
 if (__builtin_memset(data, 0x141, 12) != data) return 1;
 for (int i = 0; i < 12; ++i) if (data[i] != 65) return 2;
 unsigned char source[4] = { 3, 5, 7, 9 };
 if (__builtin_memcpy(data, source, 4) != data) return 3;
 if (__builtin_memmove(data + 1, data, 4) != data + 1) return 4;
 return data[0] == 3 && data[1] == 3 && data[2] == 5 && data[3] == 7 && data[4] == 9 && data[5] == 65 ? 17 : 5;
}
