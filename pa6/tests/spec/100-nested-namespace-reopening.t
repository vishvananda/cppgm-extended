namespace outer::inner { int value; }
namespace outer { namespace inner { int second; } }
using Item = decltype(outer::inner::value);
