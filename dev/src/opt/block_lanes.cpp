#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

#include <algorithm>

namespace opt {

using namespace lowir;

namespace {

// The most stores of one block this pass reads, and the size of the tree one
// group of stores may pack.
const size_t kBlockStores = 64;
const uint32_t kMaxNodes = 24;
const unsigned kMaxDepth = 8;
const unsigned kMaxLanes = 4;

// Block lanes at O3: adjacent stores of one block, f64 in twos and f32 in
// fours, become one sixteen-byte store when the values they store are worth
// computing in lanes. From the stores the pass walks up: lanes of one
// operation become a lane operation, loads of adjacent memory one load, and
// anything else the lanes of values computed as before. A value something
// else reads stays scalar too, so a group packs only if it removes more
// instructions than it adds. All of it happens just before the group's last
// store, where every value exists.
class BlockLanes
{
public:
	explicit BlockLanes(FunctionContext& context) : c_(context), body_(context.body) {}

	bool run();

private:
	enum class NodeKind : uint8_t
	{
		Gather,
		Splat,
		Loaded,
		Operation
	};
	struct Node
	{
		NodeKind kind = NodeKind::Gather;
		VectorOp op = VectorOp::Addpd;
		Value lanes[kMaxLanes];
		int left = -1, right = -1;
		Value made = Value::none();
	};
	struct Stored
	{
		InstId id;
		Located at;
	};

	bool block(BlockId b);
	bool group(BlockId b, const std::vector<Stored>& stores, size_t first);
	bool stores_move(const std::vector<Stored>& stores, size_t first, InstId last) const;
	bool load_moves(InstId load, const std::vector<Stored>& stores, size_t first, InstId last) const;
	int build(
		BlockId b, const Value* lanes, const std::vector<Stored>& stores, size_t first, InstId last, unsigned depth);
	int add_gather(const Value* lanes);
	bool same_block(BlockId b, Value v) const;
	int match_score(const Value* a, const Value* b) const;
	bool adjacent_loads(const Value* lanes) const;
	bool pure(const Instruction& inst) const;
	bool apart(const Instruction& store, const Located& at, uint64_t bytes, Type type, bool typed) const;
	int net_gain();
	Value lane_value(Value v);
	Value make(int node);
	Value emit(Instruction inst, Type type);
	Value vector(VectorOp op, Value a, Value b = Value::none(), int immediate = -1);

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<uint32_t> position_; // per instruction, within the block being read
	std::vector<uint32_t> uses_;
	std::vector<uint8_t> escaped_;
	std::vector<Node> nodes_;
	unsigned lanes_ = 2;
	Type lane_type_ = Type::scalar(TypeKind::F64);
	BlockId block_ = kNoId;
	InstId tail_ = kNoId;
};

bool BlockLanes::pure(const Instruction& inst) const
{
	switch (inst.op)
	{
	case Opcode::Binary:
	case Opcode::Unary:
	case Opcode::Convert:
	case Opcode::Compare:
	case Opcode::Index:
	case Opcode::Addr:
	case Opcode::Copy:
	case Opcode::Select:
	case Opcode::Vector:
		return true;
	default:
		return false;
	}
}

bool BlockLanes::apart(const Instruction& store, const Located& at, uint64_t bytes, Type type, bool typed) const
{
	const Located there = locate(body_, store.ops[1]);
	const uint64_t size = storage_bytes(store.type);
	const bool named_a = there.root.is(ValueKind::Slot) || there.root.is(ValueKind::Symbol);
	const bool named_b = at.root.is(ValueKind::Slot) || at.root.is(ValueKind::Symbol);
	if (there.root == at.root)
		return there.known && at.known &&
			(there.offset >= at.offset + static_cast<int64_t>(bytes) ||
				at.offset >= there.offset + static_cast<int64_t>(size));
	if (named_a && named_b)
		return true;
	if ((there.root.is(ValueKind::Slot) && !escaped_[there.root.index]) ||
		(at.root.is(ValueKind::Slot) && !escaped_[at.root.index]))
		return true;
	return (store.flags & IF_TYPED) != 0 && typed && typed_types_apart(store.type, type);
}

// Each store of the group but the last moves down to the last: nothing it
// passes reads or writes what it writes, and nothing it passes may leave.
bool BlockLanes::stores_move(const std::vector<Stored>& stores, size_t first, InstId last) const
{
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		const InstId id = stores[first + lane].id;
		if (id == last)
			continue;
		const Instruction& store = body_.instructions[id];
		const uint64_t bytes = storage_bytes(store.type);
		const bool typed = (store.flags & IF_TYPED) != 0;
		for (InstId i = store.next; i != last; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (pure(inst))
				continue;
			bool ok = false;
			if (inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0)
			{
				// (A load overlapping nothing the store writes.)
				Instruction read = inst;
				read.ops[1] = inst.ops[0];
				ok = apart(read, stores[first + lane].at, bytes, store.type, typed);
			}
			else if (inst.op == Opcode::Store && (inst.flags & IF_VOLATILE) == 0)
				ok = apart(inst, stores[first + lane].at, bytes, store.type, typed);
			if (!ok)
				return false;
		}
	}
	return true;
}

// A load moves down to the group's last store, before the group's stores:
// nothing after it may write what it reads, and no store of the group that
// ran before it may have.
bool BlockLanes::load_moves(InstId load, const std::vector<Stored>& stores, size_t first, InstId last) const
{
	const Instruction& read = body_.instructions[load];
	const Located at = locate(body_, read.ops[0]);
	const uint64_t bytes = storage_bytes(read.type);
	const bool typed = (read.flags & IF_TYPED) != 0;
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		const InstId id = stores[first + lane].id;
		if (position_[id] < position_[load] && !apart(body_.instructions[id], at, bytes, read.type, typed))
			return false;
	}
	for (InstId i = read.next; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (pure(inst) || (inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0))
			continue;
		if (inst.op != Opcode::Store || (inst.flags & IF_VOLATILE) != 0)
			return false;
		bool ours = false;
		for (unsigned lane = 0; lane < lanes_; ++lane)
			ours = ours || stores[first + lane].id == i;
		if (!ours && !apart(inst, at, bytes, read.type, typed))
			return false;
	}
	return true;
}

bool BlockLanes::same_block(BlockId b, Value v) const
{
	return v.is(ValueKind::Temp) && v.index < body_.temps.size() && body_.temps[v.index].def != kNoId &&
		c_.block_of[body_.temps[v.index].def] == b;
}

bool BlockLanes::adjacent_loads(const Value* lanes) const
{
	Located base;
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		const Instruction& inst = body_.instructions[body_.temps[lanes[lane].index].def];
		if (inst.op != Opcode::Load || (inst.flags & IF_VOLATILE) != 0 || inst.type != lane_type_)
			return false;
		const Located at = locate(body_, inst.ops[0]);
		if (!at.known)
			return false;
		if (lane == 0)
			base = at;
		else if (!(at.root == base.root) ||
			at.offset != base.offset + static_cast<int64_t>(lane * storage_bytes(lane_type_)))
			return false;
	}
	return true;
}

int BlockLanes::add_gather(const Value* lanes)
{
	Node node;
	node.kind = NodeKind::Splat;
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		node.lanes[lane] = lanes[lane];
		if (!(lanes[lane] == lanes[0]))
			node.kind = NodeKind::Gather;
	}
	nodes_.push_back(node);
	return static_cast<int>(nodes_.size()) - 1;
}

// How well two lists of lanes would pack, for a commutative operation's
// choice of operands.
int BlockLanes::match_score(const Value* a, const Value* b) const
{
	int score = 0;
	for (const Value* lanes : { a, b })
	{
		bool temps = true, same = true, loads = true;
		for (unsigned lane = 0; lane < lanes_; ++lane)
		{
			temps = temps && lanes[lane].is(ValueKind::Temp) && lanes[lane].index < body_.temps.size() &&
				body_.temps[lanes[lane].index].def != kNoId;
			same = same && lanes[lane] == lanes[0];
		}
		if (same)
			score += 1;
		if (!temps)
			continue;
		loads = adjacent_loads(lanes);
		bool ops = true;
		const Instruction& first = body_.instructions[body_.temps[lanes[0].index].def];
		for (unsigned lane = 1; lane < lanes_; ++lane)
		{
			const Instruction& inst = body_.instructions[body_.temps[lanes[lane].index].def];
			ops = ops && inst.op == first.op && inst.sub == first.sub;
		}
		score += loads ? 4 : ops ? 2 : 0;
	}
	return score;
}

int BlockLanes::build(
	BlockId b, const Value* lanes, const std::vector<Stored>& stores, size_t first, InstId last, unsigned depth)
{
	if (depth >= kMaxDepth || nodes_.size() >= kMaxNodes)
		return add_gather(lanes);
	bool here = true;
	for (unsigned lane = 0; lane < lanes_; ++lane)
		here = here && same_block(b, lanes[lane]);
	if (!here)
		return add_gather(lanes);
	// Each scalar has one place in the tree.
	for (const Node& known : nodes_)
		for (unsigned lane = 0; lane < lanes_ && known.kind >= NodeKind::Loaded; ++lane)
			for (unsigned other = 0; other < lanes_; ++other)
				if (known.lanes[lane] == lanes[other])
					return add_gather(lanes);
	for (unsigned lane = 1; lane < lanes_; ++lane)
		if (lanes[lane] == lanes[0])
			return add_gather(lanes);
	if (adjacent_loads(lanes))
	{
		bool moves = true;
		for (unsigned lane = 0; lane < lanes_ && moves; ++lane)
			moves = load_moves(body_.temps[lanes[lane].index].def, stores, first, last);
		if (!moves)
			return add_gather(lanes);
		const int index = add_gather(lanes);
		nodes_[index].kind = NodeKind::Loaded;
		return index;
	}
	const Instruction& head = body_.instructions[body_.temps[lanes[0].index].def];
	const bool binary = head.op == Opcode::Binary && head.type == lane_type_;
	const bool root = head.op == Opcode::Unary && head.type == lane_type_ &&
		static_cast<UnaryOp>(head.sub) == UnaryOp::Sqrt;
	if (!binary && !root)
		return add_gather(lanes);
	for (unsigned lane = 1; lane < lanes_; ++lane)
	{
		const Instruction& inst = body_.instructions[body_.temps[lanes[lane].index].def];
		if (inst.op != head.op || inst.sub != head.sub || inst.type != head.type)
			return add_gather(lanes);
	}
	const bool wide = lanes_ == 2;
	VectorOp op;
	if (root)
		op = wide ? VectorOp::Sqrtpd : VectorOp::Sqrtps;
	else
		switch (static_cast<BinaryOp>(head.sub))
		{
		case BinaryOp::Add:
			op = wide ? VectorOp::Addpd : VectorOp::Addps;
			break;
		case BinaryOp::Sub:
			op = wide ? VectorOp::Subpd : VectorOp::Subps;
			break;
		case BinaryOp::Mul:
			op = wide ? VectorOp::Mulpd : VectorOp::Mulps;
			break;
		case BinaryOp::Div:
			op = wide ? VectorOp::Divpd : VectorOp::Divps;
			break;
		default:
			return add_gather(lanes);
		}
	const int index = add_gather(lanes);
	nodes_[index].kind = NodeKind::Operation;
	nodes_[index].op = op;
	Value left[kMaxLanes], right[kMaxLanes];
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		const Instruction& inst = body_.instructions[body_.temps[lanes[lane].index].def];
		left[lane] = inst.ops[0];
		right[lane] = root ? Value::none() : inst.ops[1];
	}
	if (root)
	{
		const int child = build(b, left, stores, first, last, depth + 1);
		nodes_[index].left = child;
		return index;
	}
	// An addition or a product takes each lane's operands in either order.
	if (op == VectorOp::Addpd || op == VectorOp::Addps || op == VectorOp::Mulpd || op == VectorOp::Mulps)
		for (unsigned lane = 1; lane < lanes_; ++lane)
		{
			const int kept = match_score(left, right);
			std::swap(left[lane], right[lane]);
			if (match_score(left, right) <= kept)
				std::swap(left[lane], right[lane]);
		}
	const int l = build(b, left, stores, first, last, depth + 1);
	nodes_[index].left = l;
	const int r = build(b, right, stores, first, last, depth + 1);
	nodes_[index].right = r;
	return index;
}

// What the tree in nodes_ saves, in instructions. A scalar another reader
// needs stays, and a node none of whose scalars go is better gathered.
int BlockLanes::net_gain()
{
	std::vector<std::pair<TempId, uint32_t>> removed;
	const auto take = [&](Value v) {
		if (!v.is(ValueKind::Temp))
			return false;
		for (auto& seen : removed)
			if (seen.first == v.index)
				return ++seen.second == uses_[v.index];
		removed.push_back(std::make_pair(v.index, uint32_t(1)));
		return uses_[v.index] == 1;
	};
	int gain = static_cast<int>(lanes_) - 1;
	std::vector<uint8_t> going_lanes(nodes_.size(), 0); // bit per lane: the reader of this lane goes
	going_lanes[0] = static_cast<uint8_t>((1u << lanes_) - 1);
	for (size_t n = 0; n < nodes_.size(); ++n)
	{
		if (going_lanes[n] == 0)
			continue;
		Node& node = nodes_[n];
		int going = 0;
		uint8_t goes = 0;
		for (unsigned lane = 0; lane < lanes_; ++lane)
			if ((going_lanes[n] >> lane & 1) != 0 && take(node.lanes[lane]))
			{
				++going;
				goes |= static_cast<uint8_t>(1u << lane);
			}
		if (going == 0 && node.kind >= NodeKind::Loaded)
		{
			bool splat = true;
			for (unsigned lane = 1; lane < lanes_; ++lane)
				splat = splat && node.lanes[lane] == node.lanes[0];
			node.kind = splat ? NodeKind::Splat : NodeKind::Gather;
		}
		if (node.kind == NodeKind::Gather || node.kind == NodeKind::Splat)
		{
			gain -= node.kind == NodeKind::Splat ? 1 : static_cast<int>(lanes_) - 1;
			for (unsigned lane = 0; lane < lanes_; ++lane)
				if (!node.lanes[lane].is(ValueKind::Temp))
					gain -= 1;
			continue;
		}
		gain += going - 1;
		if (node.kind == NodeKind::Loaded)
			continue;
		// (A lane that stays still reads its operands.)
		if (node.left >= 0)
			going_lanes[node.left] = goes;
		if (node.right >= 0)
			going_lanes[node.right] = goes;
	}
	return gain;
}

Value BlockLanes::emit(Instruction inst, Type type)
{
	const InstId id = append_instruction(body_, inst);
	Value out = Value::none();
	if (!type.is_void())
	{
		const TempId result = append_temp(body_, type, id);
		body_.instructions[id].result = result;
		out = Value::temp(result);
	}
	insert_after(body_, block_, tail_, id);
	c_.block_of.resize(body_.instructions.size(), kNoId);
	c_.block_of[id] = block_;
	tail_ = id;
	return out;
}

Value BlockLanes::vector(VectorOp op, Value a, Value b, int immediate)
{
	const Type type = vector_result_type(op);
	Instruction inst = make_instruction(Opcode::Vector, type);
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = a;
	inst.ops[1] = b;
	if (immediate >= 0)
		inst.ops[2] = integer_value(body_, integer_literal(immediate));
	return emit(inst, type);
}

Value BlockLanes::lane_value(Value v)
{
	return vector(lanes_ == 2 ? VectorOp::Fromf64 : VectorOp::Fromf32, phi_input(body_, lane_type_, v));
}

Value BlockLanes::make(int index)
{
	if (!nodes_[index].made.is(ValueKind::None))
		return nodes_[index].made;
	const Node node = nodes_[index];
	Value out = Value::none();
	switch (node.kind)
	{
	case NodeKind::Splat:
		out = vector(VectorOp::Pshufd, lane_value(node.lanes[0]), Value::none(), lanes_ == 2 ? 0x44 : 0);
		break;
	case NodeKind::Gather:
		if (lanes_ == 2)
		{
			const Value low = lane_value(node.lanes[0]);
			const Value high = lane_value(node.lanes[1]);
			out = vector(VectorOp::Unpcklpd, low, high);
		}
		else
		{
			const Value a = lane_value(node.lanes[0]);
			const Value b = lane_value(node.lanes[1]);
			const Value c = lane_value(node.lanes[2]);
			const Value d = lane_value(node.lanes[3]);
			const Value low = vector(VectorOp::Unpcklps, a, b);
			const Value high = vector(VectorOp::Unpcklps, c, d);
			out = vector(VectorOp::Movlhps, low, high);
		}
		break;
	case NodeKind::Loaded:
	{
		Instruction load = make_instruction(Opcode::Load, Type::scalar(TypeKind::V128));
		const Instruction& first = body_.instructions[body_.temps[node.lanes[0].index].def];
		load.ops[0] = first.ops[0];
		load.location = first.location;
		out = emit(load, Type::scalar(TypeKind::V128));
		break;
	}
	case NodeKind::Operation:
	{
		const Value a = make(node.left);
		const Value b = node.right >= 0 ? make(node.right) : Value::none();
		out = vector(node.op, a, b);
		break;
	}
	}
	nodes_[index].made = out;
	return out;
}

bool BlockLanes::group(BlockId b, const std::vector<Stored>& stores, size_t first)
{
	InstId last = stores[first].id;
	for (unsigned lane = 1; lane < lanes_; ++lane)
		if (position_[stores[first + lane].id] > position_[last])
			last = stores[first + lane].id;
	if (!stores_move(stores, first, last))
		return false;
	Value lanes[kMaxLanes];
	bool useful = false;
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		lanes[lane] = body_.instructions[stores[first + lane].id].ops[0];
		useful = useful || same_block(b, lanes[lane]);
	}
	if (!useful)
		return false;
	nodes_.clear();
	build(b, lanes, stores, first, last, 0);
	if (net_gain() <= 0 || nodes_[0].kind < NodeKind::Loaded)
		return false;
	block_ = b;
	tail_ = previous_in_block(body_, b, last);
	const Value out = make(0);
	const Instruction& low = body_.instructions[stores[first].id];
	Instruction store = make_instruction(Opcode::Store, Type::scalar(TypeKind::V128));
	store.ops[0] = out;
	store.ops[1] = low.ops[1];
	store.location = low.location;
	emit(store, Type::scalar(TypeKind::Void));
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		const InstId id = stores[first + lane].id;
		unlink_after(body_, b, previous_in_block(body_, b, id), id);
		c_.block_of[id] = kNoId;
	}
	return true;
}

bool BlockLanes::block(BlockId b)
{
	std::vector<Stored> stores;
	uint32_t position = 0;
	for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		position_[i] = position++;
		if (inst.op == Opcode::Store && (inst.flags & IF_VOLATILE) == 0 &&
			(inst.type.kind() == TypeKind::F64 || inst.type.kind() == TypeKind::F32) && stores.size() < kBlockStores)
		{
			const Located at = locate(body_, inst.ops[1]);
			if (at.known)
				stores.push_back(Stored{ i, at });
		}
	}
	if (stores.size() < 2)
		return false;
	std::sort(stores.begin(), stores.end(), [](const Stored& a, const Stored& b) {
		if (a.at.root.kind != b.at.root.kind)
			return a.at.root.kind < b.at.root.kind;
		if (a.at.root.index != b.at.root.index)
			return a.at.root.index < b.at.root.index;
		return a.at.offset < b.at.offset;
	});
	for (size_t i = 0; i + 1 < stores.size();)
	{
		const Type type = body_.instructions[stores[i].id].type;
		const unsigned lanes = type.kind() == TypeKind::F64 ? 2 : 4;
		const int64_t step = type.kind() == TypeKind::F64 ? 8 : 4;
		bool run = i + lanes <= stores.size();
		for (unsigned lane = 1; lane < lanes && run; ++lane)
			run = stores[i + lane].at.root == stores[i].at.root &&
				stores[i + lane].at.offset == stores[i].at.offset + lane * step &&
				body_.instructions[stores[i + lane].id].type == type;
		if (!run)
		{
			++i;
			continue;
		}
		lanes_ = lanes;
		lane_type_ = type;
		// (A group changes the block's order: it is read again.)
		if (group(b, stores, i))
			return true;
		++i;
	}
	return false;
}

bool BlockLanes::run()
{
	// (Only a block with two floating stores costs anything below.)
	bool any = false;
	for (const Instruction& inst : body_.instructions)
		any = any || (inst.op == Opcode::Store && inst.type.is_float());
	if (!any)
		return false;
	position_.assign(body_.instructions.size(), 0);
	count_uses(body_, uses_);
	escaped_ = escaped_slots(body_);
	map_instruction_blocks(body_, c_.block_of);
	bool changed = false;
	for (BlockId b : body_.layout)
		for (unsigned round = 0; round < kBlockStores && block(b); ++round)
		{
			position_.resize(body_.instructions.size(), 0);
			changed = true;
		}
	if (changed)
		c_.invalidate_block_of();
	return changed;
}

} // namespace

bool pack_block_lanes(FunctionContext& context)
{
	BlockLanes pass(context);
	return pass.run();
}

} // namespace opt
