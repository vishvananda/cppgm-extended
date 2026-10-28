#include "lowir/text_writer.h"

#include <cstring>

#include "lowir/literals.h"
#include "lowir/local_names.h"
#include "ir_symbol_model.h"
#include "support/internal_error.h"

namespace lowir {

namespace {

// A vocabulary spelling with its length, for appending without strlen.
struct Spelled
{
	const char* text;
	uint32_t length;
};

// The spellings of the N enumerators of E (lowir_model.h), measured once,
// before main: the tables below hold only constant data until then.
template<typename E, unsigned N>
struct SpelledTable
{
	Spelled entries[N];

	SpelledTable()
	{
		for (unsigned i = 0; i < N; ++i)
		{
			entries[i].text = lowir_model::spelling(static_cast<E>(i));
			entries[i].length = static_cast<uint32_t>(std::strlen(entries[i].text));
		}
	}

	const Spelled& operator[](unsigned index) const { return entries[index]; }
};

const SpelledTable<Opcode, kOpcodeCount> kHeads;
const SpelledTable<UnaryOp, kUnaryOpCount> kUnaryWords;
const SpelledTable<BinaryOp, kBinaryOpCount> kBinaryWords;
const SpelledTable<ComparePredicate, kComparePredicateCount> kCompareWords;
const SpelledTable<ConvertOp, kConvertOpCount> kConvertWords;

// The size whose eightbytes a value's classes name (append_object_classes).
uint64_t classes_bytes(Type type)
{
	if (type.is_object())
		return type.bytes;
	return type.kind() == TypeKind::I128 || type.kind() == TypeKind::F80 ? 16 : (integer_bits(type.kind()) + 7) / 8;
}

class Writer
{
public:
	// With a sink, the text is written to it whenever the buffer `out`
	// reaches kFlushBytes, so the buffer stays bounded.
	Writer(const Program& program, std::string& out, std::FILE* sink = nullptr)
		: program_(program), out_(out), body_(nullptr), sink_(sink), failed_(false), definitions_(nullptr),
		  names_(program)
	{}

	void write();
	// The function-definition phase comes from a stream (the bodies may be
	// released); set before write().
	void use_definitions(DefinitionStream* definitions) { definitions_ = definitions; }
	void function_definition(const Symbol& symbol);
	// Writes the buffered text to the sink; false if a write failed.
	bool flush();

private:
	void text(const char* s) { out_ += s; }
	void debug_info();
	void debug_name(NameId name);
	// A word of the vocabulary and the space after it.
	void word(const char* s)
	{
		out_ += s;
		out_.push_back(' ');
	}
	void spelled(const Spelled& w) { out_.append(w.text, w.length); }
	void head(Opcode op)
	{
		spelled(kHeads[static_cast<unsigned>(op)]);
		out_.push_back(' ');
	}
	void number(uint64_t value);
	void signed_number(int64_t value);
	void name(char prefix, NameId id, uint32_t fallback);
	void symbol(SymbolId id) { name('@', program_.symbols[id].name, id); }
	void type(Type t);
	void value(Value v);
	// A value as an operand of `context` (its literal spelled for it).
	void operand(Value v, TypeKind context);
	void metadata(const SymbolMetadata* symbol, const BoundaryMetadata* boundary, const Signature* signature = nullptr);
	void classes_item(bool& open, MetadataKey key, uint8_t classes, Type type);
	void key(bool& open, MetadataKey key);
	void item(bool& open, MetadataKey key, const char* value);
	// A definition's parameters are the body's (`definition`), spelled as
	// its uses are.
	void parameters(const Signature& signature, bool definition = false);
	void argument_classes(const Instruction& inst, bool& open);
	void constant(const Constant& c, TypeKind context);
	void address(SymbolId target, int64_t addend);
	void location(const DebugLocation& location);
	void global_declaration(const Symbol& symbol);
	void function_declaration(const Symbol& symbol);
	void global_definition(const Symbol& symbol);
	void instruction(const Instruction& inst);
	bool write_value_instruction(const Instruction& inst);
	bool write_memory_instruction(const Instruction& inst);
	void write_effect(const Instruction& inst);
	void write_eh(const Instruction& inst);
	void write_call(const Instruction& inst);
	void operand_list(uint32_t first, uint32_t count);
	void argument_list(const Instruction& call);

	const Program& program_;
	std::string& out_;
	const FunctionBody* body_;
	std::FILE* sink_;
	bool failed_;
	DefinitionStream* definitions_;
	std::string scratch_;
	LocalNames names_;
};

const size_t kFlushBytes = 1 << 20;

bool Writer::flush()
{
	if (sink_ != nullptr && !out_.empty())
	{
		failed_ = failed_ || std::fwrite(out_.data(), 1, out_.size(), sink_) != out_.size();
		out_.clear();
	}
	return !failed_;
}

void Writer::number(uint64_t value)
{
	char digits[24];
	int count = 0;
	do
	{
		digits[count++] = static_cast<char>('0' + value % 10);
		value /= 10;
	} while (value != 0);
	while (count > 0)
		out_.push_back(digits[--count]);
}

void Writer::signed_number(int64_t value)
{
	if (value < 0)
	{
		out_.push_back('-');
		number(0 - static_cast<uint64_t>(value));
	}
	else
		number(static_cast<uint64_t>(value));
}

void Writer::name(char prefix, NameId id, uint32_t fallback)
{
	out_.push_back(prefix);
	if (id != 0)
		out_.append(program_.names.spelling(id), program_.names.length(id));
	else
		number(fallback);
}

void Writer::type(Type t)
{
	if (!t.is_object())
	{
		text(spelling(t.kind()));
		return;
	}
	text(spelling(TypeKind::Obj));
	out_.push_back('<');
	number(t.bytes);
	out_.push_back('x');
	number(static_cast<uint64_t>(1) << t.align_log2);
	out_.push_back('>');
}

void Writer::value(Value v)
{
	switch (v.kind)
	{
	case ValueKind::Temp:
		out_.push_back('%');
		names_.temp(out_, v.index);
		break;
	case ValueKind::Slot:
		out_.push_back('$');
		names_.slot(out_, v.index);
		break;
	case ValueKind::Symbol:
		symbol(v.index);
		break;
	case ValueKind::Block:
		out_.push_back('^');
		names_.block(out_, v.index);
		break;
	case ValueKind::Integer:
		append_integer_literal(out_, body_->literals[v.index]);
		break;
	case ValueKind::Float:
		append_float_literal(out_, v.float_kind, body_->literals[v.index], program_.object_facts, true);
		break;
	case ValueKind::None:
		break;
	}
}

void Writer::operand(Value v, TypeKind context)
{
	if (v.kind == ValueKind::Integer && float_rank(context) != 0)
		append_float_literal(
			out_, context, integer_as_float(context, body_->literals[v.index]), program_.object_facts, false);
	else if (v.kind == ValueKind::Integer)
		append_integer_literal(out_, body_->literals[v.index], context);
	else if (v.kind == ValueKind::Float)
		append_float_literal(
			out_, v.float_kind, body_->literals[v.index], program_.object_facts, float_rank(context) == 0);
	else
		value(v);
}

void Writer::key(bool& open, MetadataKey key)
{
	text(open ? ", " : " [");
	open = true;
	text(spelling(key));
	out_.push_back('=');
}

void Writer::item(bool& open, MetadataKey key, const char* value)
{
	this->key(open, key);
	text(value);
}

void Writer::classes_item(bool& open, MetadataKey key, uint8_t classes, Type type)
{
	scratch_.clear();
	append_object_classes(scratch_, classes, classes_bytes(type));
	item(open, key, scratch_.c_str());
}

void Writer::metadata(const SymbolMetadata* symbol, const BoundaryMetadata* boundary, const Signature* signature)
{
	bool open = false;
	if (boundary != nullptr)
	{
		if (boundary->arity != 0)
			item(open, MetadataKey::Arity, spelling(boundary->arity));
		if (boundary->effects != 0)
			item(open, MetadataKey::Effects, spelling(boundary->effects));
		if (boundary->unwind != 0)
			item(open, MetadataKey::Unwind, spelling(boundary->unwind));
		if (boundary->returns != 0)
			item(open, MetadataKey::Return, spelling(boundary->returns));
		if (signature != nullptr && signature->boundary.result_classes != 0)
			classes_item(open, MetadataKey::ResultClasses, signature->boundary.result_classes, signature->result);
	}
	if (symbol != nullptr)
	{
		if (symbol->storage != 0)
			item(open, MetadataKey::Storage, spelling(symbol->storage));
		if (symbol->section != 0)
			item(open, MetadataKey::Section, program_.spelling(symbol->section));
		if (symbol->role != 0)
			item(open, MetadataKey::Role, spelling(symbol->role));
		if (symbol->linkage != 0)
			item(open, MetadataKey::Linkage, spelling(symbol->linkage));
		if (symbol->binding != 0)
			item(open, MetadataKey::Binding, spelling(symbol->binding));
		if ((symbol->flags & SF_REPLACEABLE) != 0)
			item(open, MetadataKey::Replaceable, metadata_yes_spelling());
		if (symbol->object != 0)
			item(open, MetadataKey::Object, program_.spelling(symbol->object));
		if (symbol->tls_for != kNoId)
		{
			item(open, MetadataKey::TlsFor, "");
			this->symbol(symbol->tls_for);
		}
		// The flags from keep_alias to no_inline (replaceable is above).
		for (unsigned flag = 0; flag < 6u; ++flag)
		{
			if ((symbol->flags & (1u << flag)) != 0)
				item(open, symbol_flag_key(static_cast<SymbolFlag>(1u << flag)), metadata_yes_spelling());
		}
	}
	if (open)
		out_.push_back(']');
}

void Writer::parameters(const Signature& signature, bool definition)
{
	out_.push_back('(');
	const Parameter* parameter = program_.parameters_of(signature);
	for (uint32_t i = 0; i < signature.parameter_count; ++i)
	{
		if (i != 0)
			text(", ");
		out_.push_back('%');
		if (definition)
			names_.temp(out_, i);
		else
			names_.boundary_parameter(out_, signature, i);
		text(" : ");
		type(parameter[i].type);
		bool open = false;
		if (parameter[i].pass != 0)
			item(open, MetadataKey::Pass, spelling(parameter[i].pass));
		if (parameter[i].alias != 0)
			item(open, MetadataKey::Alias, spelling(parameter[i].alias));
		if (parameter[i].object_bytes != 0)
		{
			key(open, MetadataKey::ObjectBytes);
			number(parameter[i].object_bytes);
		}
		if (parameter[i].classes != 0)
		{
			key(open, MetadataKey::Classes);
			append_object_classes(out_, parameter[i].classes, classes_bytes(parameter[i].type));
		}
		if (open)
			out_.push_back(']');
	}
	text(") -> ");
	type(signature.result);
}

void Writer::constant(const Constant& c, TypeKind context)
{
	if (c.kind == ValueKind::Integer && float_rank(context) != 0)
		append_float_literal(out_, context, integer_as_float(context, c.bits), program_.object_facts, false);
	else if (c.kind == ValueKind::Float)
		append_float_literal(out_, c.float_kind, c.bits, program_.object_facts, float_rank(context) == 0);
	else
		append_integer_literal(out_, c.bits, context);
}

void Writer::address(SymbolId target, int64_t addend)
{
	head(Opcode::Addr);
	symbol(target);
	if (addend > 0)
	{
		text(" + ");
		signed_number(addend);
	}
	else if (addend < 0)
	{
		text(" - ");
		number(0 - static_cast<uint64_t>(addend));
	}
}

void Writer::location(const DebugLocation& location)
{
	if (!location.present())
		return;
	text(" !dbg(");
	text(program_.spelling(location.file));
	text(", ");
	number(location.line);
	text(", ");
	number(location.column);
	if (location.scope != 0)
	{
		text(", ");
		number(location.scope);
	}
	out_.push_back(')');
}

// ---------------------------------------------------------------------------
// Top level

void Writer::write()
{
	const SymbolKind phases[] = { SymbolKind::GlobalDeclaration, SymbolKind::FunctionDeclaration,
		SymbolKind::GlobalDefinition, SymbolKind::FunctionDefinition };
	bool written = false;
	for (SymbolKind phase : phases)
	{
		bool phase_written = false;
		if (phase == SymbolKind::FunctionDefinition && definitions_ != nullptr)
		{
			if (!definitions_->empty())
			{
				if (written)
					out_.push_back('\n');
				flush();
				failed_ = failed_ || !definitions_->copy_to(sink_);
				written = true;
			}
			continue;
		}
		for (SymbolId id : program_.order)
		{
			const Symbol& symbol = program_.symbols[id];
			if (symbol.kind != phase)
				continue;
			if (written && !phase_written)
				out_.push_back('\n');
			phase_written = true;
			switch (phase)
			{
			case SymbolKind::GlobalDeclaration:
				global_declaration(symbol);
				break;
			case SymbolKind::FunctionDeclaration:
				function_declaration(symbol);
				break;
			case SymbolKind::GlobalDefinition:
				global_definition(symbol);
				break;
			default:
				function_definition(symbol);
				break;
			}
			if (out_.size() >= kFlushBytes)
				flush();
		}
		written = written || phase_written;
	}
	debug_info();
	for (const ObjectAlias& alias : program_.aliases)
	{
		word(spelling(Keyword::Alias));
		word(spelling(Keyword::Object));
		text(program_.spelling(alias.object));
		text(" = ");
		symbol(alias.target);
		out_.push_back('\n');
	}
}

void Writer::global_declaration(const Symbol& symbol)
{
	const Global& global = program_.globals[symbol.detail];
	text("declare global ");
	name('@', symbol.name, 0);
	if (global.typed)
	{
		text(" : ");
		type(global.type);
	}
	metadata(&symbol.metadata, nullptr);
	out_.push_back('\n');
}

void Writer::debug_name(NameId id)
{
	out_.push_back('"');
	const char* name = id != 0 ? program_.spelling(id) : "";
	for (const char* p = name; *p != 0; ++p)
	{
		if (*p == '\\' || *p == '"' || *p == '\n' || *p == '\r' || *p == '\t')
			out_.push_back('\\');
		out_.push_back(*p == '\n' ? 'n' : *p == '\r' ? 'r' : *p == '\t' ? 't' : *p);
	}
	out_.push_back('"');
}

void Writer::debug_info()
{
	if (!program_.debug)
		return;
	const DebugInfo& debug = *program_.debug;
	for (uint32_t id = 1; id < debug.types.size(); ++id)
	{
		const DebugType& type = debug.types[id];
		word(spelling(Keyword::DebugType));
		for (uint64_t value : { uint64_t(id), uint64_t(type.kind), uint64_t(type.encoding), uint64_t(type.parent),
				 uint64_t(type.type), uint64_t(type.flags), type.bytes, type.bit_offset, type.count })
		{
			number(value);
			out_.push_back(' ');
		}
		signed_number(static_cast<int64_t>(type.constant));
		out_.push_back(' ');
		debug_name(type.name);
		out_.push_back('\n');
	}
	for (uint32_t id = 1; id < debug.scopes.size(); ++id)
	{
		word(spelling(Keyword::DebugScope));
		number(id);
		out_.push_back(' ');
		name('@', debug.scopes[id].owner, 0);
		out_.push_back(' ');
		number(debug.scopes[id].parent);
		out_.push_back('\n');
	}
	for (SymbolId function = 0; function < debug.functions.size(); ++function)
	{
		if (function >= program_.symbols.size() || program_.symbols[function].kind != SymbolKind::FunctionDefinition)
			continue;
		for (const DebugLocal& local : debug.functions[function].locals)
		{
			word(spelling(Keyword::DebugLocal));
			symbol(function);
			out_.push_back(' ');
			number(local.slot);
			out_.push_back(' ');
			number(local.type);
			out_.push_back(' ');
			number(local.scope);
			out_.push_back(' ');
			debug_name(local.name);
			location(local.declaration);
			out_.push_back('\n');
		}
	}
}

void Writer::function_declaration(const Symbol& symbol)
{
	const Signature& signature = program_.signatures[program_.functions[symbol.detail].signature];
	text("declare function ");
	name('@', symbol.name, 0);
	parameters(signature);
	metadata(&symbol.metadata, &signature.boundary, &signature);
	out_.push_back('\n');
}

void Writer::global_definition(const Symbol& symbol)
{
	const Global& global = program_.globals[symbol.detail];
	text("global ");
	name('@', symbol.name, 0);
	// A structured global's storage type, when it states one (lowir.md
	// "Structured Global Data Initializers").
	if (global.init != GlobalInit::Data || global.typed)
	{
		text(" : ");
		type(global.type);
	}
	metadata(&symbol.metadata, nullptr);
	text(" = ");
	switch (global.init)
	{
	case GlobalInit::Zero:
		text(spelling(Keyword::Zero));
		out_.push_back('\n');
		return;
	case GlobalInit::Constant:
		constant(global.constant, global.type.kind());
		out_.push_back('\n');
		return;
	case GlobalInit::Address:
		address(global.address, global.addend);
		out_.push_back('\n');
		return;
	default:
		break;
	}
	text("{\n");
	for (uint32_t i = 0; i < global.item_count; ++i)
	{
		const DataItem& data = program_.data_items[global.first_item + i];
		text("  ");
		if (data.kind == DataItemKind::Zero)
		{
			word(spelling(Keyword::Zero));
			signed_number(data.amount);
		}
		else
		{
			type(data.type);
			out_.push_back(' ');
			if (data.kind == DataItemKind::Address)
				address(data.address, data.amount);
			else
				constant(data.constant, data.type.kind());
		}
		out_.push_back('\n');
		if (out_.size() >= kFlushBytes)
			flush();
	}
	text("}\n");
}

void Writer::function_definition(const Symbol& symbol)
{
	const Function& function = program_.functions[symbol.detail];
	const Signature& signature = program_.signatures[function.signature];
	body_ = function.body.get();
	names_.reset(*body_);
	text("function ");
	name('@', symbol.name, 0);
	parameters(signature, true);
	metadata(&symbol.metadata, &signature.boundary, &signature);
	location(function.location);
	text(" {\n");
	for (SlotId slot = 0; slot < body_->slots.size(); ++slot)
	{
		text("  slot $");
		names_.slot(out_, slot);
		text(" : ");
		type(body_->slots[slot].type);
		out_.push_back('\n');
	}
	for (size_t i = 0; i < body_->layout.size(); ++i)
	{
		if (i != 0 || !body_->slots.empty())
			out_.push_back('\n');
		const BlockId block = body_->layout[i];
		text("  block ^");
		names_.block(out_, block);
		text(":\n");
		for (InstId id = body_->blocks[block].first; id != kNoId; id = body_->instructions[id].next)
			instruction(body_->instructions[id]);
	}
	text("}\n");
	body_ = nullptr;
}

// ---------------------------------------------------------------------------
// Instructions

void Writer::operand_list(uint32_t first, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		if (i != 0)
			text(", ");
		value(body_->operands[first + i]);
	}
}

void Writer::argument_list(const Instruction& call)
{
	// Each argument as its parameter's operand; one to `...`, or of a call
	// whose boundary is not known yet, as written (a floating literal kept
	// floating). An integer literal to an integer parameter keeps the bits
	// the call passes.
	const Signature* boundary = program_.call_boundary(call);
	const Parameter* parameters = boundary != nullptr ? program_.parameters_of(*boundary) : nullptr;
	for (uint32_t i = 0; i < call.extra_count; ++i)
	{
		if (i != 0)
			text(", ");
		const Value argument = body_->operands[call.extra_first + i];
		const TypeKind kind = boundary != nullptr && i < boundary->parameter_count ? parameters[i].type.kind()
																				   : TypeKind::Void;
		if (argument.kind == ValueKind::Integer && float_rank(kind) != 0)
			append_float_literal(
				out_, kind, integer_as_float(kind, body_->literals[argument.index]), program_.object_facts, true);
		else
			value(argument);
	}
}

void Writer::instruction(const Instruction& inst)
{
	text("    ");
	if (inst.result != kNoId)
	{
		value(Value::temp(inst.result));
		text(" = ");
	}
	if (!write_value_instruction(inst) && !write_memory_instruction(inst))
		write_effect(inst);
	location(body_->location_of(inst));
	out_.push_back('\n');
	if (out_.size() >= kFlushBytes)
		flush();
}

bool Writer::write_value_instruction(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Const:
		head(inst.op);
		break;
	case Opcode::Copy:
		head(inst.op);
		break;
	case Opcode::VaArg:
		head(inst.op);
		break;
	case Opcode::Exception:
		head(inst.op);
		type(inst.type);
		return true;
	case Opcode::ExceptionSelector:
		head(inst.op);
		type(inst.type);
		return true;
	case Opcode::StackAlloc:
		head(inst.op);
		value(inst.ops[0]);
		return true;
	case Opcode::Call:
		write_call(inst);
		return true;
	case Opcode::Phi:
		head(inst.op);
		type(inst.type);
		text(" [");
		for (uint32_t i = 0; i < inst.extra_count; i += 2)
		{
			if (i != 0)
				text(", ");
			value(body_->operands[inst.extra_first + i]);
			text(": ");
			operand(body_->operands[inst.extra_first + i + 1], inst.type.kind());
		}
		out_.push_back(']');
		return true;
	case Opcode::Select:
		head(inst.op);
		type(inst.type);
		out_.push_back(' ');
		operand(inst.ops[0], TypeKind::I64);
		text(", ");
		operand(inst.ops[1], inst.type.kind());
		text(", ");
		operand(inst.ops[2], inst.type.kind());
		return true;
	case Opcode::Unary:
	case Opcode::Binary:
	case Opcode::Compare:
		head(inst.op);
		spelled(inst.op == Opcode::Unary    ? kUnaryWords[inst.sub]
				: inst.op == Opcode::Binary ? kBinaryWords[inst.sub]
											: kCompareWords[inst.sub]);
		for (const InstructionFlag fact : { IF_NO_WRAP, IF_EXACT })
		{
			if (inst.op == Opcode::Binary && (inst.flags & fact) != 0)
			{
				out_.push_back(' ');
				text(spelling(fact));
			}
		}
		out_.push_back(' ');
		type(inst.type);
		out_.push_back(' ');
		operand(inst.ops[0], inst.type.kind());
		if (inst.op != Opcode::Unary)
		{
			text(", ");
			operand(inst.ops[1], inst.type.kind());
		}
		return true;
	case Opcode::Convert:
		head(inst.op);
		spelled(kConvertWords[inst.sub]);
		out_.push_back(' ');
		type(inst.type);
		out_.push_back(' ');
		type(convert_source_type(inst));
		out_.push_back(' ');
		operand(inst.ops[0], convert_source_type(inst).kind());
		return true;
	default:
		return false;
	}
	// const, copy, va_arg: `<type> <value>` (va_arg's a va_list address).
	type(inst.type);
	out_.push_back(' ');
	operand(inst.ops[0], inst.op == Opcode::VaArg ? TypeKind::Ptr : inst.type.kind());
	if (inst.op == Opcode::VaArg)
	{
		bool open = false;
		argument_classes(inst, open);
		if (open)
			out_.push_back(']');
	}
	return true;
}

bool Writer::write_memory_instruction(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Addr:
		head(inst.op);
		value(inst.ops[0]);
		return true;
	case Opcode::Load:
	case Opcode::Store:
		head(inst.op);
		for (const InstructionFlag fact : { IF_VOLATILE, IF_TYPED })
		{
			if ((inst.flags & fact) != 0)
				word(spelling(fact));
		}
		type(inst.type);
		out_.push_back(' ');
		operand(inst.ops[0], inst.op == Opcode::Store ? inst.type.kind() : TypeKind::Ptr);
		if (inst.op == Opcode::Store)
		{
			text(", ");
			operand(inst.ops[1], TypeKind::Ptr);
		}
		return true;
	case Opcode::Index:
		head(inst.op);
		type(inst.type);
		if (inst.sub != 0)
		{
			bool open = false;
			item(open, MetadataKey::Projection, spelling(static_cast<IndexProjectionKind>(inst.sub)));
			out_.push_back(']');
		}
		out_.push_back(' ');
		value(inst.ops[0]);
		text(", ");
		value(inst.ops[1]);
		return true;
	case Opcode::AtomicLoad:
		head(inst.op);
		break;
	case Opcode::AtomicStore:
		head(inst.op);
		break;
	case Opcode::AtomicAddFetch:
		head(inst.op);
		break;
	case Opcode::AtomicExchange:
		head(inst.op);
		break;
	case Opcode::AtomicCompareExchange:
		head(inst.op);
		break;
	default:
		return false;
	}
	// The atomic operations: `<type> <operands>, <order>[, <failure order>]`:
	// addresses, then (but for a load) the value or the expected value's
	// address and the desired value.
	type(inst.type);
	out_.push_back(' ');
	operand(inst.ops[0], inst.op == Opcode::AtomicStore ? inst.type.kind() : TypeKind::Ptr);
	for (int i = 1; i < 3; ++i)
	{
		if (!inst.ops[i].is(ValueKind::None))
		{
			text(", ");
			const bool address = inst.op == Opcode::AtomicStore || (inst.op == Opcode::AtomicCompareExchange && i == 1);
			operand(inst.ops[i], address ? TypeKind::Ptr : inst.type.kind());
		}
	}
	text(", ");
	number(static_cast<unsigned>(inst.order));
	if (inst.op == Opcode::AtomicCompareExchange)
	{
		text(", ");
		number(inst.sub);
	}
	return true;
}

// Object mode: the classes of the instruction's object arguments to `...`
// (a call's `classes=<argument>:<classes>` items) or of a va_arg's object
// (`classes=<classes>`), in a metadata group `open` may have begun.
void Writer::argument_classes(const Instruction& inst, bool& open)
{
	if (!program_.object_facts || body_->argument_classes.empty())
		return;
	const InstId id = static_cast<InstId>(&inst - body_->instructions.data());
	for (const ArgumentClasses& entry : body_->argument_classes)
	{
		if (entry.call != id)
			continue;
		key(open, MetadataKey::Classes);
		if (inst.op == Opcode::Call)
		{
			number(entry.argument);
			out_.push_back(':');
		}
		append_object_classes(out_, entry.classes, 0);
	}
}

void Writer::write_call(const Instruction& inst)
{
	head(inst.op);
	type(inst.type);
	out_.push_back(' ');
	value(inst.ops[0]);
	out_.push_back('(');
	argument_list(inst);
	out_.push_back(')');
	bool open = false;
	if ((inst.flags & IF_COPY_ELISION) != 0)
	{
		item(open, MetadataKey::Elision, elision_copy_spelling());
	}
	argument_classes(inst, open);
	if (open)
		out_.push_back(']');
	if ((inst.flags & IF_SIGNATURE) != 0)
	{
		const Signature& signature = program_.signatures[static_cast<SignatureId>(inst.immediate)];
		out_.push_back(' ');
		word(spelling(Keyword::As));
		parameters(signature);
		metadata(nullptr, &signature.boundary, &signature);
	}
}

void Writer::write_eh(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::EhTry:
		head(inst.op);
		value(inst.ops[0]);
		return;
	case Opcode::EhCleanup:
		text(spelling(inst.op));
		if (!inst.ops[0].is(ValueKind::None))
		{
			out_.push_back(' ');
			value(inst.ops[0]);
		}
		return;
	case Opcode::EhCatch:
		head(inst.op);
		value(inst.ops[0]);
		break;
	case Opcode::EhFilter:
		text(spelling(inst.op));
		if (inst.extra_count != 0)
		{
			out_.push_back(' ');
			operand_list(inst.extra_first, inst.extra_count);
		}
		if ((inst.flags & IF_SELECTOR) != 0)
		{
			text(inst.extra_count != 0 ? ", " : " ");
			signed_number(inst.immediate);
		}
		return;
	case Opcode::EhCatchAll:
		text(spelling(inst.op));
		break;
	default:
		text(spelling(inst.op));
		return;
	}
	if ((inst.flags & IF_SELECTOR) != 0)
	{
		text(", ");
		signed_number(inst.immediate);
	}
}

void Writer::write_effect(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::AtomicThreadFence:
	case Opcode::AtomicSignalFence:
		head(inst.op);
		number(static_cast<unsigned>(inst.order));
		return;
	case Opcode::VaStart:
		head(inst.op);
		value(inst.ops[0]);
		return;
	case Opcode::CopyObject:
	case Opcode::ZeroInit:
		head(inst.op);
		number(static_cast<uint64_t>(inst.immediate));
		out_.push_back('x');
		number(static_cast<uint64_t>(1) << inst.sub);
		out_.push_back(' ');
		value(inst.ops[0]);
		if (inst.op == Opcode::CopyObject)
		{
			text(", ");
			value(inst.ops[1]);
		}
		return;
	case Opcode::Throw:
		head(inst.op);
		type(inst.type);
		out_.push_back(' ');
		operand(inst.ops[0], inst.type.kind());
		return;
	case Opcode::Resume:
		text(spelling(inst.op));
		return;
	case Opcode::Unreachable:
		text(spelling(inst.op));
		return;
	case Opcode::Jump:
		head(inst.op);
		value(inst.ops[0]);
		return;
	case Opcode::Branch:
		head(inst.op);
		operand(inst.ops[0], TypeKind::I64);
		text(", ");
		value(inst.ops[1]);
		text(", ");
		value(inst.ops[2]);
		return;
	case Opcode::Switch:
		head(inst.op);
		value(inst.ops[0]);
		text(", ");
		value(inst.ops[1]);
		for (uint32_t i = 0; i < inst.extra_count; i += 2)
		{
			text(", ");
			value(body_->operands[inst.extra_first + i]);
			out_.push_back(':');
			value(body_->operands[inst.extra_first + i + 1]);
		}
		return;
	case Opcode::Return:
		head(inst.op);
		type(inst.type);
		if (!inst.ops[0].is(ValueKind::None))
		{
			out_.push_back(' ');
			operand(inst.ops[0], inst.type.kind());
		}
		return;
	default:
		write_eh(inst);
		return;
	}
}

} // namespace

bool write_program_file(const Program& program, const std::string& path)
{
	std::FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr)
		return false;
	std::string buffer;
	buffer.reserve(kFlushBytes + 4096);
	Writer writer(program, buffer, file);
	writer.write();
	const bool written = writer.flush();
	return std::fclose(file) == 0 && written;
}

void write_program(const Program& program, std::string& out)
{
	Writer writer(program, out);
	writer.write();
}

DefinitionStream::~DefinitionStream()
{
	if (spill_ != nullptr)
		std::fclose(spill_);
}

void DefinitionStream::append(const Program& program, SymbolId function)
{
	starts_.push_back(spilled_ + buffer_.size());
	Writer writer(program, buffer_);
	writer.function_definition(program.symbols[function]);
	if (buffer_.size() < kFlushBytes)
		return;
	if (spill_ == nullptr)
		spill_ = std::tmpfile();
	failed_ = failed_ || spill_ == nullptr || std::fwrite(buffer_.data(), 1, buffer_.size(), spill_) != buffer_.size();
	spilled_ += buffer_.size();
	buffer_.clear();
}

bool DefinitionStream::copy_range(std::FILE* file, uint64_t first, uint64_t last)
{
	// From the spill file, then from the buffer that follows it.
	if (first < spilled_)
	{
		const uint64_t end = last < spilled_ ? last : spilled_;
		if (std::fseek(spill_, static_cast<long>(first), SEEK_SET) != 0)
			return false;
		char chunk[1 << 16];
		for (uint64_t left = end - first; left != 0;)
		{
			const size_t count = left < sizeof chunk ? static_cast<size_t>(left) : sizeof chunk;
			if (std::fread(chunk, 1, count, spill_) != count || std::fwrite(chunk, 1, count, file) != count)
				return false;
			left -= count;
		}
		first = end;
	}
	if (first >= last)
		return true;
	const size_t at = static_cast<size_t>(first - spilled_);
	const size_t count = static_cast<size_t>(last - first);
	return std::fwrite(buffer_.data() + at, 1, count, file) == count;
}

bool DefinitionStream::copy_to(std::FILE* file)
{
	if (failed_)
		return false;
	if (!order_.empty())
	{
		const uint64_t total = spilled_ + buffer_.size();
		for (const uint32_t append : order_)
		{
			const uint64_t last = append + 1 < starts_.size() ? starts_[append + 1] : total;
			if (!copy_range(file, starts_[append], last))
				return false;
		}
		return true;
	}
	if (spill_ != nullptr)
	{
		std::rewind(spill_);
		char chunk[1 << 16];
		size_t count = 0;
		while ((count = std::fread(chunk, 1, sizeof chunk, spill_)) != 0)
		{
			if (std::fwrite(chunk, 1, count, file) != count)
				return false;
		}
		if (std::ferror(spill_))
			return false;
	}
	return std::fwrite(buffer_.data(), 1, buffer_.size(), file) == buffer_.size();
}

bool write_program_file(const Program& program, DefinitionStream& definitions, const std::string& path)
{
	std::FILE* file = std::fopen(path.c_str(), "wb");
	if (file == nullptr)
		return false;
	std::string buffer;
	buffer.reserve(kFlushBytes + 4096);
	Writer writer(program, buffer, file);
	writer.use_definitions(&definitions);
	writer.write();
	const bool written = writer.flush();
	return std::fclose(file) == 0 && written;
}

} // namespace lowir
