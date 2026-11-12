#include <stdio.h>
#include <sys/stat.h>
int main() {
 struct stat info;
 if (stat("input.txt", &info) != 0 || info.st_size != 6) return 1;
 FILE* input = fopen("input.txt", "rb");
 if (!input) return 2;
 char bytes[8] = {};
 unsigned count = fread(bytes, 1, sizeof bytes, input);
 if (fclose(input) != 0 || count != 6 || bytes[0] != 'h' || bytes[5] != '!') return 3;
 FILE* output = fopen("output.bin", "wb");
 if (!output) return 4;
 if (fwrite(bytes, 1, count, output) != count || fseek(output, 2, SEEK_SET) != 0) return 5;
 if (fputc('X', output) != 'X' || fclose(output) != 0) return 6;
 input = fopen("output.bin", "rb");
 if (!input) return 7;
 if (fseek(input, -1, SEEK_END) != 0 || fgetc(input) != '!' || ftell(input) != 6) return 8;
 if (fclose(input) != 0) return 9;
 puts("file streams pass");
 return 0;
}
