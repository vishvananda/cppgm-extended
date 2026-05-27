#include <memory>
#include <string>
#include <utility>

struct AliasTemplateDecl {};
struct ClassTemplateDecl {};

typedef std::pair<const std::string, AliasTemplateDecl *> AliasPair;
typedef std::pair<const std::string, ClassTemplateDecl *> ClassPair;

int main()
{
  AliasPair alias_pair = AliasPair(std::string("a"), (AliasTemplateDecl *)0);
  ClassPair class_pair = ClassPair(std::string("c"), (ClassTemplateDecl *)0);
  return std::pointer_traits<const AliasPair *>::pointer_to(alias_pair) == &alias_pair &&
                 std::pointer_traits<const ClassPair *>::pointer_to(class_pair) == &class_pair ?
      0 :
      1;
}
