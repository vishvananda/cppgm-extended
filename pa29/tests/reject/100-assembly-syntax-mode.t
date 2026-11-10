extern "C" int bad_syntax() { asm(".intel_syntax invalid\nnop"); return 1; }
