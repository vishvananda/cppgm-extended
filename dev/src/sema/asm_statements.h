#pragma once

// Typed GNU operands and their frontend-neutral assembly packet.
namespace assembly {
struct Packet;
}

namespace sema {
struct ConstValue;

struct AsmProgram
{
	const assembly::Packet* packet = nullptr;
	const ConstValue* addresses = nullptr;
};

} // namespace sema
