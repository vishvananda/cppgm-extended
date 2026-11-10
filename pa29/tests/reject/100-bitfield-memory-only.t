struct Fields { unsigned value : 7; };
void rejected(Fields& p) { asm("" : "=m"(p.value)); }
