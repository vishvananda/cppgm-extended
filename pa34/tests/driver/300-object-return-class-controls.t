// Integer, SSE, mixed and memory return classes preserve every payload field.
using U=unsigned long;
struct B1 { unsigned char bytes[1]; };
struct B2 { unsigned char bytes[2]; };
struct B3 { unsigned char bytes[3]; };
struct B4 { unsigned char bytes[4]; };
struct B5 { unsigned char bytes[5]; };
struct B6 { unsigned char bytes[6]; };
struct B7 { unsigned char bytes[7]; };
struct B8 { unsigned char bytes[8]; };
struct B9 { unsigned char bytes[9]; };
struct B10 { unsigned char bytes[10]; };
struct B12 { unsigned char bytes[12]; };
struct B16 { unsigned char bytes[16]; };
struct B17 { unsigned char bytes[17]; };
struct B24 { unsigned char bytes[24]; };
struct F{float x;}; struct Q{double x;}; struct M{double x;U key;}; struct P{U lo,hi;}; struct L{U word[3];};
extern "C" __attribute__((noinline)) B1 f1(B1 x,unsigned mask){for(unsigned j=0;j<1;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B2 f2(B2 x,unsigned mask){for(unsigned j=0;j<2;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B3 f3(B3 x,unsigned mask){for(unsigned j=0;j<3;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B4 f4(B4 x,unsigned mask){for(unsigned j=0;j<4;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B5 f5(B5 x,unsigned mask){for(unsigned j=0;j<5;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B6 f6(B6 x,unsigned mask){for(unsigned j=0;j<6;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B7 f7(B7 x,unsigned mask){for(unsigned j=0;j<7;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B8 f8(B8 x,unsigned mask){for(unsigned j=0;j<8;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B9 f9(B9 x,unsigned mask){for(unsigned j=0;j<9;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B10 f10(B10 x,unsigned mask){for(unsigned j=0;j<10;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B12 f12(B12 x,unsigned mask){for(unsigned j=0;j<12;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B16 f16(B16 x,unsigned mask){for(unsigned j=0;j<16;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B17 f17(B17 x,unsigned mask){for(unsigned j=0;j<17;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) B24 f24(B24 x,unsigned mask){for(unsigned j=0;j<24;++j)x.bytes[j]^=static_cast<unsigned char>(mask+j*7);return x;}
extern "C" __attribute__((noinline)) F fF(F x,U key){x.x+=float(key&7);return x;}
extern "C" __attribute__((noinline)) Q fQ(Q x,U key){x.x+=double(key&31);return x;}
extern "C" __attribute__((noinline)) M fM(M x,U key){x.x+=double(key&31);x.key^=key;return x;}
extern "C" __attribute__((noinline)) P fP(P x,U key){x.lo^=key;x.hi+=key;return x;}
extern "C" __attribute__((noinline)) L fL(L x,U key){x.word[0]^=key;x.word[1]+=key;x.word[2]-=key;return x;}

int main(){U seed=1,h=0;for(unsigned i=0;i<8192;++i){seed=seed*6364136223846793005UL+1;unsigned mask=unsigned(seed>>32);{B1 x={};for(unsigned j=0;j<1;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B1 r=f1(x,mask);for(unsigned j=0;j<1;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B2 x={};for(unsigned j=0;j<2;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B2 r=f2(x,mask);for(unsigned j=0;j<2;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B3 x={};for(unsigned j=0;j<3;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B3 r=f3(x,mask);for(unsigned j=0;j<3;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B4 x={};for(unsigned j=0;j<4;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B4 r=f4(x,mask);for(unsigned j=0;j<4;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B5 x={};for(unsigned j=0;j<5;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B5 r=f5(x,mask);for(unsigned j=0;j<5;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B6 x={};for(unsigned j=0;j<6;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B6 r=f6(x,mask);for(unsigned j=0;j<6;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B7 x={};for(unsigned j=0;j<7;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B7 r=f7(x,mask);for(unsigned j=0;j<7;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B8 x={};for(unsigned j=0;j<8;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B8 r=f8(x,mask);for(unsigned j=0;j<8;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B9 x={};for(unsigned j=0;j<9;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B9 r=f9(x,mask);for(unsigned j=0;j<9;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B10 x={};for(unsigned j=0;j<10;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B10 r=f10(x,mask);for(unsigned j=0;j<10;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B12 x={};for(unsigned j=0;j<12;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B12 r=f12(x,mask);for(unsigned j=0;j<12;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B16 x={};for(unsigned j=0;j<16;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B16 r=f16(x,mask);for(unsigned j=0;j<16;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B17 x={};for(unsigned j=0;j<17;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B17 r=f17(x,mask);for(unsigned j=0;j<17;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}{B24 x={};for(unsigned j=0;j<24;++j)x.bytes[j]=static_cast<unsigned char>((seed>>(j&7))^j);B24 r=f24(x,mask);for(unsigned j=0;j<24;++j){unsigned char v=static_cast<unsigned char>(x.bytes[j]^static_cast<unsigned char>(mask+j*7));if(r.bytes[j]!=v)return 1;h=h*31+r.bytes[j];}}float a=float(seed&65535)/4;double b=double(seed&1048575)/8;
{F x={a};F y=fF(x,seed);if(y.x!=a+float(seed&7))return 2;h=h*31+U(y.x*4);}
{Q x={b};Q y=fQ(x,seed);if(y.x!=b+double(seed&31))return 3;h=h*31+U(y.x*8);}
{M x={b,~seed};M y=fM(x,seed);if(y.x!=b+double(seed&31)||y.key!=(~seed^seed))return 4;h=h*31+y.key;}
{P x={~seed,seed*3};P y=fP(x,seed);if(y.lo!=(~seed^seed)||y.hi!=seed*4)return 5;h=h*31+y.hi;}
{L x={{seed*3,~seed,seed*7}};L y=fL(x,seed);if(y.word[0]!=(seed*3^seed)||y.word[1]!=~seed+seed||y.word[2]!=seed*6)return 6;h=h*31+y.word[2];}
}return h != 12353697668242404058UL;}
