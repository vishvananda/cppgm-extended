// Power-of-two alternatives, conversions, modular rounding and failed-proof controls.

typedef unsigned long U;
extern "C" unsigned div32(unsigned x,unsigned k) { unsigned d;switch(k&3) {case 0:d=1;break;case 1:d=2;break;case 2:d=8;break;default:d=16;break;}return x/d; }
extern "C" unsigned mod32(unsigned x,unsigned k) { unsigned d=(k&1)?((k&2)?4:32):((k&2)?1:256); return x%d; }
extern "C" U div64(U x,unsigned k) { U d=(k&1)?((k&2)?(U(1)<<32):8):((k&2)?1:(U(1)<<63)); return x/d; }
extern "C" U mod64(U x,unsigned k) { U d=(k&1)?((k&2)?(U(1)<<32):8):((k&2)?1:(U(1)<<63)); return x%d; }
extern "C" unsigned shift32(unsigned x,unsigned k) { return x/(1U<<(k&31)); }
extern "C" U shift64(U x,U k) { return x%(U(1)<<(k&63)); }
extern "C" unsigned unknown32(unsigned x,unsigned k) { return x/((k&1)?3:8); }
extern "C" U unknown64(U x,unsigned k) { return x%((k&1)?7:32); }
extern "C" unsigned loop32(unsigned x,unsigned k) { unsigned d=1; for(unsigned i=0;i<(k&7);++i) { d=(i&1)?d:4; } return x/d; }
extern "C" unsigned truncdiv(unsigned x,unsigned k) { U a=(k&1)?8:32;return x/(unsigned)a; }
extern "C" U widenmod(U x,unsigned k) { unsigned char a=(k&1)?16:64;return x%a; }
extern "C" unsigned mixedtrunc(unsigned x,unsigned k) { U a=(k&1)?8:((U(1)<<32)|3);return x/(unsigned)a; }
extern "C" unsigned round32(unsigned x,unsigned k) { unsigned d=(k&1)?4:8;unsigned b=x!=0;return ((x-b)/d+b)*d; }
extern "C" U round64(U x,unsigned k) { U d=(k&1)?16:64;U b=x!=0;return d*(b+(x-b)/d); }
extern "C" unsigned roundfixed(unsigned x) { return ((x-(x!=0))/8+(x!=0))*8; }
extern "C" unsigned roundshift(unsigned x,unsigned k) { unsigned d=1U<<(k&31);unsigned b=x!=0;return ((x-b)/d+b)*d; }
extern "C" unsigned roundbad(unsigned x,unsigned k) { unsigned d=(k&1)?3:8;unsigned b=x!=0;return ((x-b)/d+b)*d; }
extern "C" unsigned roundflag(unsigned x,unsigned k) { unsigned d=8,b=k&1;return ((x-b)/d+b)*d; }
extern "C" unsigned roundinverse(unsigned x) { unsigned b=x==0;return ((x-b)/8+b)*8; }
extern "C" long signed_div(long x,unsigned k) { return x/((k&1)?4L:8L); }

int main()
{
    const unsigned edges[] = {0, 1, ~0U, ~0U - 1};
    const unsigned divisors[] = {1, 2, 8, 16};
    U state = 0x123fefaa31UL;
    for (unsigned i = 0; i < 1024; ++i)
    {
        state = state * 6364136223846793005UL + 1;
        unsigned x = i < 4 ? edges[i] : (unsigned)state;
        for (unsigned k = 0; k < 129; ++k)
        {
            U d = (k & 1) ? ((k & 2) ? (U(1) << 32) : 8) : ((k & 2) ? 1 : (U(1) << 63));
            unsigned m = (k & 1) ? ((k & 2) ? 4 : 32) : ((k & 2) ? 1 : 256);
            unsigned loop_divisor = (k & 7) ? 4 : 1;
            if (div32(x, k) != x / divisors[k & 3] || mod32(x, k) != x % m ||
                div64(state, k) != state / d || mod64(state, k) != state % d ||
                shift32(x, k) != x / (1U << (k & 31)) || shift64(state, k) != state % (U(1) << (k & 63)) ||
                unknown32(x, k) != x / ((k & 1) ? 3 : 8) || unknown64(state, k) != state % ((k & 1) ? 7 : 32) ||
                loop32(x, k) != x / loop_divisor || signed_div((long)state, k) != (long)state / ((k & 1) ? 4L : 8L))
                return 1;
            if (truncdiv(x, k) != x / ((k & 1) ? 8 : 32) || widenmod(state, k) != state % ((k & 1) ? 16 : 64) ||
                mixedtrunc(x, k) != x / ((k & 1) ? 8 : 3))
                return 2;
            // Wider addition followed by truncation is an independent oracle for modular alignment.
            U a = (k & 1) ? 4 : 8;
            unsigned expected = (unsigned)((U(x) + a - 1) & ~(a - 1));
            U b = (k & 1) ? 16 : 64;
            U c = U(1) << (k & 31);
            if (round32(x, k) != expected || round64(state, k) != ((state + b - 1) & ~(b - 1)) ||
                roundfixed(x) != (unsigned)((U(x) + 7) & ~U(7)) ||
                roundshift(x, k) != (unsigned)((U(x) + c - 1) & ~(c - 1)))
                return 3;
            unsigned nonpower = (k & 1) ? 3 : 8;
            unsigned flag = k & 1;
            if (roundbad(x, k) != ((x - (x != 0)) / nonpower + (x != 0)) * nonpower ||
                roundflag(x, k) != ((x - flag) / 8 + flag) * 8 ||
                roundinverse(x) != ((x - (x == 0)) / 8 + (x == 0)) * 8)
                return 4;
        }
    }
    return 0;
}
