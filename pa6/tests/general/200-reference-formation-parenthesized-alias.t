// N3485 8.3.2p5/p6: direct reference formation versus permitted collapsing. expect exit 0
typedef int &R;extern R (&value);
