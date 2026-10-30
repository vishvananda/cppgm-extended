void f(){unsigned a=0,b=0,c=0,d=0;asm volatile("cpuid %0":"+a"(a),"+b"(b),"+c"(c),"+d"(d));}
