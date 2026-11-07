// N3485 8.3.2p5/p6: direct reference formation versus permitted collapsing. expect exit 0
typedef int &L;typedef int &&R;
void f(L&,L&&,R&,R&&);
using LR=L&&;using RL=R&;extern LR a;extern RL b;
