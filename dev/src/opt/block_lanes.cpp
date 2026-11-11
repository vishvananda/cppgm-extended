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
// store, where every value exists. A group that does not pay alone may pay
// with a later group of the block that reads the same lanes (the two halves
// of a butterfly read one loaded pair): the later group reuses the vectors
// the first made.
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
	bool reload(BlockId b, const std::vector<Stored>& stores, const std::vector<size_t>& runs);
	bool quiet(const Instruction& inst) const;
	int plan(BlockId b, const std::vector<Stored>& stores, size_t first, InstId& last);
	bool group(BlockId b, const std::vector<Stored>& stores, const std::vector<size_t>& runs, size_t run);
	void pack(BlockId b, const std::vector<Stored>& stores, size_t first, InstId last, int root);
	bool stores_move(const std::vector<Stored>& stores, size_t first, InstId last) const;
	bool load_moves(InstId load, const std::vector<Stored>& stores, size_t first, InstId last) const;
	int build(
		BlockId b, const Value* lanes, const std::vector<Stored>& stores, size_t first, InstId last, unsigned depth);
	int add_gather(const Value* lanes);
	bool same_block(BlockId b, Value v) const;
	int match_score(const Value* a, const Value* b) const;
	bool adjacent_loads(const Value* lanes) const;
	bool pure(const Instruction& inst) const;
	bool apart(const Instruction& store, Value address, const Located& at, uint64_t bytes, Type type, bool typed) const;
	int net_gain(const int* roots, size_t count);
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
	size_t shared_ = 0; // nodes_ below this belong to an earlier group a later one may reuse
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

bool BlockLanes::apart(
	const Instruction& store, Value address, const Located& at, uint64_t bytes, Type type, bool typed) const
{
	const Located there = locate_steps(body_, store.ops[1]);
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
	if ((store.flags & IF_TYPED) != 0 && typed && typed_types_apart(store.type, type))
		return true;
	return strided_apart(strided_address(body_, store.ops[1]), size, strided_address(body_, address), bytes);
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
				ok = apart(read, store.ops[1], stores[first + lane].at, bytes, store.type, typed);
			}
			else if (inst.op == Opcode::Store && (inst.flags & IF_VOLATILE) == 0)
				ok = apart(inst, store.ops[1], stores[first + lane].at, bytes, store.type, typed);
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
	const Located at = locate_steps(body_, read.ops[0]);
	const uint64_t bytes = storage_bytes(read.type);
	const bool typed = (read.flags & IF_TYPED) != 0;
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		const InstId id = stores[first + lane].id;
		if (position_[id] < position_[load] && !apart(body_.instructions[id], read.ops[0], at, bytes, read.type, typed))
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
		if (!ours && !apart(inst, read.ops[0], at, bytes, read.type, typed))
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
		const Located at = locate_steps(body_, inst.ops[0]);
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
	if (depth >= kMaxDepth || nodes_.size() - shared_ >= kMaxNodes)
		return add_gather(lanes);
	bool here = true;
	for (unsigned lane = 0; lane < lanes_; ++lane)
		here = here && same_block(b, lanes[lane]);
	if (!here)
		return add_gather(lanes);
	// Each scalar has one place in the tree. An earlier group's node of
	// these very lanes is reused: its vector exists where this group packs.
	for (size_t n = 0; n < shared_; ++n)
	{
		bool same = nodes_[n].kind >= NodeKind::Loaded;
		for (unsigned lane = 0; lane < lanes_ && same; ++lane)
			same = nodes_[n].lanes[lane] == lanes[lane];
		if (same)
			return static_cast<int>(n);
	}
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

// What the trees in nodes_ from `roots` save, in instructions. A scalar
// another reader needs stays, and a node none of whose scalars go is better
// gathered. A node two trees share counts the readers of both; the nodes are
// read parents first (a later tree's nodes read only earlier nodes, which
// its own parents reach once theirs are read).
int BlockLanes::net_gain(const int* roots, size_t count)
{
	std::vector<uint32_t> removed(nodes_.size() * kMaxLanes, 0); // per node and lane: readers that go
	std::vector<uint32_t> parents(nodes_.size(), 0), seen(nodes_.size(), 0);
	std::vector<uint8_t> needed(nodes_.size(), 0); // a root, or read by a lane operation
	for (const Node& node : nodes_)
		if (node.kind == NodeKind::Operation)
		{
			if (node.left >= 0)
				++parents[node.left];
			if (node.right >= 0)
				++parents[node.right];
		}
	std::vector<int> ready;
	int gain = 0;
	for (size_t r = 0; r < count; ++r)
	{
		gain += static_cast<int>(lanes_) - 1;
		for (unsigned lane = 0; lane < lanes_; ++lane)
			++removed[roots[r] * kMaxLanes + lane];
		needed[roots[r]] = 1;
		if (parents[roots[r]] == 0)
			ready.push_back(roots[r]);
	}
	while (!ready.empty())
	{
		const int n = ready.back();
		ready.pop_back();
		Node& node = nodes_[n];
		const bool operation = node.kind == NodeKind::Operation;
		if (needed[n] == 0)
		{
			// (Nothing makes it: its children are not read through it.)
			for (const int child : { node.left, node.right })
				if (operation && child >= 0 && ++seen[child] == parents[child])
					ready.push_back(child);
			continue;
		}
		int going = 0;
		uint8_t goes = 0;
		for (unsigned lane = 0; lane < lanes_; ++lane)
		{
			const Value v = node.lanes[lane];
			const uint32_t r = removed[n * kMaxLanes + lane];
			if (r != 0 && v.is(ValueKind::Temp) && r == uses_[v.index])
			{
				++going;
				goes |= static_cast<uint8_t>(1u << lane);
			}
		}
		if (going == 0 && node.kind >= NodeKind::Loaded)
		{
			bool splat = true;
			for (unsigned lane = 1; lane < lanes_; ++lane)
				splat = splat && node.lanes[lane] == node.lanes[0];
			node.kind = splat ? NodeKind::Splat : NodeKind::Gather;
		}
		const bool reads = node.kind == NodeKind::Operation;
		if (node.kind == NodeKind::Gather || node.kind == NodeKind::Splat)
		{
			gain -= node.kind == NodeKind::Splat ? 1 : static_cast<int>(lanes_) - 1;
			for (unsigned lane = 0; lane < lanes_; ++lane)
				if (!node.lanes[lane].is(ValueKind::Temp))
					gain -= 1;
		}
		else
			gain += going - 1;
		// (A lane that stays still reads its operands.) A child is read once
		// all its parents are.
		for (const int child : { node.left, node.right })
		{
			if (!operation || child < 0)
				continue;
			if (reads)
			{
				needed[child] = 1;
				for (unsigned lane = 0; lane < lanes_; ++lane)
					if ((goes >> lane & 1) != 0)
						++removed[child * kMaxLanes + lane];
			}
			if (++seen[child] == parents[child])
				ready.push_back(child);
		}
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

// Builds the tree of the group of stores from `first` after nodes_ (the
// earlier group's, below shared_): its root, or -1 if the group cannot pack.
// `last` is the group's last store.
int BlockLanes::plan(BlockId b, const std::vector<Stored>& stores, size_t first, InstId& last)
{
	last = stores[first].id;
	for (unsigned lane = 1; lane < lanes_; ++lane)
		if (position_[stores[first + lane].id] > position_[last])
			last = stores[first + lane].id;
	if (!stores_move(stores, first, last))
		return -1;
	Value lanes[kMaxLanes];
	bool useful = false;
	for (unsigned lane = 0; lane < lanes_; ++lane)
	{
		lanes[lane] = body_.instructions[stores[first + lane].id].ops[0];
		useful = useful || same_block(b, lanes[lane]);
	}
	if (!useful)
		return -1;
	return build(b, lanes, stores, first, last, 0);
}

void BlockLanes::pack(BlockId b, const std::vector<Stored>& stores, size_t first, InstId last, int root)
{
	block_ = b;
	tail_ = previous_in_block(body_, b, last);
	const Value out = make(root);
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
}

// The group of runs[run], alone or with another group whose stores all come
// after its own and whose tree reuses some of its nodes.
bool BlockLanes::group(BlockId b, const std::vector<Stored>& stores, const std::vector<size_t>& runs, size_t run)
{
	const size_t first = runs[run];
	nodes_.clear();
	shared_ = 0;
	InstId last = kNoId;
	const int root = plan(b, stores, first, last);
	if (root < 0)
		return false;
	const std::vector<Node> built = nodes_;
	if (net_gain(&root, 1) > 0 && nodes_[root].kind >= NodeKind::Loaded)
	{
		pack(b, stores, first, last, root);
		return true;
	}
	for (size_t other = 0; other < runs.size(); ++other)
	{
		const size_t second = runs[other];
		const Type type = body_.instructions[stores[second].id].type;
		bool after = type == lane_type_;
		for (unsigned lane = 0; lane < lanes_ && after; ++lane)
			after = position_[stores[second + lane].id] > position_[last];
		if (!after)
			continue;
		nodes_ = built;
		shared_ = built.size();
		InstId later = kNoId;
		const int next = plan(b, stores, second, later);
		bool reuses = false;
		for (size_t n = shared_; n < nodes_.size() && next >= 0; ++n)
			for (const int child : { nodes_[n].left, nodes_[n].right })
				reuses = reuses ||
					(nodes_[n].kind == NodeKind::Operation && child >= 0 && static_cast<size_t>(child) < shared_);
		if (next < 0 || !reuses)
			continue;
		const int roots[2] = { root, next };
		if (net_gain(roots, 2) <= 0 || nodes_[root].kind < NodeKind::Loaded || nodes_[next].kind < NodeKind::Loaded)
			continue;
		pack(b, stores, first, last, root);
		pack(b, stores, second, later, next);
		return true;
	}
	return false;
}

// An instruction that writes no memory and leaves its block only as its
// terminator.
bool BlockLanes::quiet(const Instruction& inst) const
{
	return pure(inst) || inst.op == Opcode::Load || inst.op == Opcode::Phi || inst.op == Opcode::Jump ||
		inst.op == Opcode::Branch;
}

// A one-block loop's phi that stands for a location a group stores: a load
// of the location before the loop gives its first value, and the loop's last
// store there its next. The loop reads the location instead, so that the
// group's lanes are loads of adjacent memory. Nothing between that load and
// the loop writes memory, and nothing after that store may write the
// location.
bool BlockLanes::reload(BlockId b, const std::vector<Stored>& stores, const std::vector<size_t>& runs)
{
	const Instruction& last = body_.instructions[body_.blocks[b].last];
	const InstId first = body_.blocks[b].first;
	if (last.op != Opcode::Branch || (last.ops[1].index != b && last.ops[2].index != b) ||
		body_.instructions[first].op != Opcode::Phi)
		return false;
	Cfg& cfg = c_.ensure_cfg_light();
	BlockId pre = kNoId;
	for (uint32_t k = cfg.pred_first[b]; k < cfg.pred_first[b + 1]; ++k)
		if (cfg.preds[k] != b)
		{
			if (pre != kNoId && pre != cfg.preds[k])
				return false;
			pre = cfg.preds[k];
		}
	if (pre == kNoId)
		return false;
	for (InstId prev = kNoId, i = first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		prev = i, i = body_.instructions[i].next)
	{
		const Instruction phi = body_.instructions[i];
		if (!phi.type.is_float() || phi.extra_count != 4)
			continue;
		const Value before = phi_incoming(body_, phi, pre), next = phi_incoming(body_, phi, b);
		// The last store of the next value, one of a group's.
		InstId store = kNoId;
		for (const size_t run : runs)
			for (unsigned lane = 0; lane < (body_.instructions[stores[run].id].type.kind() == TypeKind::F64 ? 2u : 4u);
				++lane)
			{
				const Instruction& s = body_.instructions[stores[run + lane].id];
				if (s.ops[0] == next && s.type == phi.type &&
					(store == kNoId || position_[stores[run + lane].id] > position_[store]))
					store = stores[run + lane].id;
			}
		if (store == kNoId || !before.is(ValueKind::Temp) || before.index < body_.parameter_count)
			continue;
		const Instruction written = body_.instructions[store];
		const Located at = locate_steps(body_, written.ops[1]);
		const bool typed = (written.flags & IF_TYPED) != 0;
		bool ok = !same_block(b, written.ops[1]);
		for (InstId j = written.next; j != kNoId && ok; j = body_.instructions[j].next)
		{
			const Instruction& inst = body_.instructions[j];
			ok = quiet(inst) ||
				(inst.op == Opcode::Store && (inst.flags & IF_VOLATILE) == 0 &&
					apart(inst, written.ops[1], at, storage_bytes(written.type), written.type, typed));
		}
		// The load before the loop, and the way from it to the loop.
		const InstId load = body_.temps[before.index].def;
		if (!ok || load == kNoId)
			continue;
		const Instruction read = body_.instructions[load];
		const Located from = locate_steps(body_, read.ops[0]);
		if (read.op != Opcode::Load || (read.flags & IF_VOLATILE) != 0 || read.type != phi.type ||
			typed != ((read.flags & IF_TYPED) != 0) || !at.known || !from.known || !(from.root == at.root) ||
			from.offset != at.offset)
			continue;
		BlockId p = pre;
		for (unsigned steps = 0; ok && p != c_.block_of[load]; ++steps)
		{
			for (InstId j = body_.blocks[p].first; j != kNoId && ok; j = body_.instructions[j].next)
				ok = quiet(body_.instructions[j]);
			ok = ok && steps < kMaxDepth && cfg.pred_count(p) == 1;
			if (ok)
				p = cfg.preds[cfg.pred_first[p]];
		}
		for (InstId j = read.next; j != kNoId && ok; j = body_.instructions[j].next)
			ok = quiet(body_.instructions[j]);
		if (!ok)
			continue;
		// The phi goes; its readers read the location at the block's start.
		InstId after = prev;
		for (InstId j = i; j != kNoId && body_.instructions[j].op == Opcode::Phi; j = body_.instructions[j].next)
			after = j;
		unlink_after(body_, b, prev, i);
		if (after == i)
			after = prev;
		Instruction again = read;
		again.next = kNoId;
		again.ops[0] = written.ops[1];
		const InstId id = append_instruction(body_, again);
		const TempId result = append_temp(body_, phi.type, id);
		body_.instructions[id].result = result;
		insert_after(body_, b, after, id);
		for (Instruction& inst : body_.instructions)
			for_each_operand(body_, inst, [&](Value& v) {
				if (v == Value::temp(phi.result))
					v = Value::temp(result);
			});
		map_instruction_blocks(body_, c_.block_of);
		count_uses(body_, uses_);
		return true;
	}
	return false;
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
			const Located at = locate_steps(body_, inst.ops[1]);
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
	// The runs of adjacent stores of one type that may form a group.
	std::vector<size_t> runs;
	for (size_t i = 0; i + 1 < stores.size(); ++i)
	{
		const Type type = body_.instructions[stores[i].id].type;
		const unsigned lanes = type.kind() == TypeKind::F64 ? 2 : 4;
		const int64_t step = type.kind() == TypeKind::F64 ? 8 : 4;
		bool run = i + lanes <= stores.size();
		for (unsigned lane = 1; lane < lanes && run; ++lane)
			run = stores[i + lane].at.root == stores[i].at.root &&
				stores[i + lane].at.offset == stores[i].at.offset + lane * step &&
				body_.instructions[stores[i + lane].id].type == type;
		if (run)
			runs.push_back(i);
	}
	if (!runs.empty() && reload(b, stores, runs))
		return true;
	for (size_t r = 0; r < runs.size(); ++r)
	{
		const Type type = body_.instructions[stores[runs[r]].id].type;
		lanes_ = type.kind() == TypeKind::F64 ? 2 : 4;
		lane_type_ = type;
		// (A group changes the block's order: it is read again.)
		if (group(b, stores, runs, r))
			return true;
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
