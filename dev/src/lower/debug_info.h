#pragma once

#include <string>
#include "lowir/debug_info.h"
#include "support/pointer_map.h"

namespace sema {
struct Type;
struct Scope;
struct Entity;
struct ClassEntity;
struct VariableEntity;
}
namespace lower {
class ProgramLowering;

// One unit's canonical semantic identities -> owned optional LowIR facts.
// The collector never completes a class or replays lookup/layout work.
class DebugInfo
{
public:
	explicit DebugInfo(ProgramLowering& lowering);
	void local(lowir::SymbolId function, lowir::SlotId slot, const sema::VariableEntity* variable, uint32_t location);
	uint32_t type(const sema::Type* type);
	void finish(const sema::Scope* root);

private:
	uint32_t add(const lowir::DebugType& record);
	uint32_t scope(const sema::Scope* scope);
	void members(uint32_t id, const sema::ClassEntity* entity);
	std::string name(const sema::Type* type, bool qualified = false);
	std::string entity_name(const sema::Entity* entity, bool qualified);
	lowir::NameId intern(const std::string& text);
	ProgramLowering& lowering_;
	lowir::DebugInfo& info_;
	PointerMap types_;
	PointerMap scopes_;
	PointerMap aliases_;
};
} // namespace lower
