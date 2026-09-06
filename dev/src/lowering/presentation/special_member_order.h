#pragma once

namespace cppgm
{
namespace lowering
{
namespace ir
{
struct Program;
}
namespace presentation
{

// The generated-definition order the LowIR document states for special member
// families: for one class, copy forms precede move forms (copy constructor
// before move constructor, copy assignment before move assignment);
// constructor entry points appear base before complete; destructor entry
// points appear base, deleting, complete.  Definitions are emitted in demand
// order, which puts a move assignment first when the move is demanded
// first.  This pass permutes the members of each family among the positions
// they already occupy and leaves every other definition where it is.
void OrderSpecialMemberFamilies(ir::Program* program);

}
}
}
