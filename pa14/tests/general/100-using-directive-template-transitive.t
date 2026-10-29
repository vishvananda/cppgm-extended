namespace Candidates { template<class T> int choose(int) { return 42; } }
namespace Bridge { using namespace Candidates; int choose(int, int) { return 7; } }
using namespace Bridge;
int choose() { return 1; }
namespace InnerValue {
    using namespace Candidates;
    int choose = 1;
    int check() { int x=2; return (choose < x) > (0,1); }
}
namespace InlineValue {
    int choose = 1;
    inline namespace Child { using namespace Candidates; }
    int check() { int x=2; return (choose < x) > (0,1); }
}
namespace QualifiedValue { using namespace Candidates; int choose = 1; }
int check_qualified() { int x=2; return (QualifiedValue::choose < x) > (0,1); }
int check_local() { int choose=1, x=2; return (choose < x) > (0,1); }
int main() {
    if (choose() != 1 || choose(1,2) != 7 || choose<int>(0) != 42) return 1;
    return InnerValue::check() || InlineValue::check() || check_qualified() || check_local();
}
