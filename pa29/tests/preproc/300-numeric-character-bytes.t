#if '\xff' != -1 || '\200' != -128 || '\x7f' != 127
#error GNU byte character values must sign extend
#endif
byte_character_values_ok
