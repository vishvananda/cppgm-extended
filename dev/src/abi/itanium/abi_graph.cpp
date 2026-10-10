// Hash-consed construction of the canonical ABI graph.

#include "abi/itanium/abi_graph.h"

#include <algorithm>
#include <cstring>

#include "support/internal_error.h"

namespace abi_mangle {

namespace {

const std::uint32_t kInitialIndexSize = 1024;

std::uint32_t mix(std::uint32_t hash, std::uint32_t value)
{
	hash ^= value;
	hash *= 16777619u;
	return hash ^ (hash >> 15);
}

AbiNodeData make_data(AbiNodeKind kind)
{
	AbiNodeData data;
	std::memset(&data, 0, sizeof data);
	data.kind = kind;
	return data;
}

} // namespace

AbiGraph::AbiGraph() : arena_start_(arena_.mark()), intern_requests_(0), intern_hits_(0)
{
	// Reset rewinds to just after the first block's start, which it keeps.
	arena_.allocate(1, 1);
	arena_start_ = arena_.mark();
	nodes_.push_back(make_data(ABI_NODE_NONE));
	hashes_.push_back(0);
	index_.assign(kInitialIndexSize, ABI_NONE);
}

void AbiGraph::reset()
{
	nodes_.resize(1);
	hashes_.resize(1);
	lists_.clear();
	texts_.clear();
	index_.assign(kInitialIndexSize, ABI_NONE);
	tag_memo_.clear();
	tag_marks_.clear();
	tag_written_marks_.clear();
	for (std::vector<std::uint32_t>& memo : tag_set_memo_)
		memo.clear();
	tag_sets_.clear();
	tag_set_items_.clear();
	tag_epoch_ = 0;
	underived_.clear();
	arena_.rewind(arena_start_);
}

long long AbiGraph::value(AbiNode literal) const
{
	const AbiNodeData& data = nodes_[literal];
	const unsigned long long bits = (static_cast<unsigned long long>(data.c) << 32) | data.b;
	return static_cast<long long>(bits);
}

std::size_t AbiGraph::bytes_reserved() const
{
	return arena_.bytes_reserved() + nodes_.capacity() * sizeof(AbiNodeData) +
		hashes_.capacity() * sizeof(std::uint32_t) + lists_.capacity() * sizeof(AbiNode) +
		texts_.capacity() * sizeof(AbiText) + index_.capacity() * sizeof(AbiNode);
}

std::uint32_t AbiGraph::text_hash(const char* text, std::size_t length) const
{
	std::uint32_t hash = 2166136261u ^ ABI_NODE_STRING;
	for (std::size_t i = 0; i < length; ++i)
	{
		hash ^= static_cast<unsigned char>(text[i]);
		hash *= 16777619u;
	}
	return hash;
}

std::uint32_t AbiGraph::hash_of(const AbiNodeData& data, const AbiNode* items, std::size_t count) const
{
	std::uint32_t hash = 2166136261u;
	hash = mix(hash,
		data.kind | (static_cast<std::uint32_t>(data.flags) << 8) | (static_cast<std::uint32_t>(data.code) << 16));
	hash = mix(hash, data.a);
	hash = mix(hash, data.b);
	hash = mix(hash, data.c);
	for (std::size_t i = 0; i < count; ++i)
		hash = mix(hash, items[i]);
	return mix(hash, static_cast<std::uint32_t>(count));
}

bool AbiGraph::same(AbiNode id, const AbiNodeData& data, const AbiNode* items, std::size_t count) const
{
	const AbiNodeData& existing = nodes_[id];
	if (existing.kind != data.kind || existing.flags != data.flags || existing.code != data.code ||
		existing.a != data.a || existing.b != data.b || existing.c != data.c || existing.list_size != count)
		return false;
	return count == 0 || std::memcmp(lists_.data() + existing.list_begin, items, count * sizeof(AbiNode)) == 0;
}

void AbiGraph::grow_index()
{
	std::vector<AbiNode> grown(index_.size() * 2, ABI_NONE);
	const std::uint32_t mask = static_cast<std::uint32_t>(grown.size() - 1);
	for (AbiNode id : index_)
	{
		if (id == ABI_NONE)
			continue;
		std::uint32_t position = hashes_[id] & mask;
		while (grown[position] != ABI_NONE)
			position = (position + 1) & mask;
		grown[position] = id;
	}
	index_.swap(grown);
}

AbiNode AbiGraph::string(const char* text, std::size_t length)
{
	++intern_requests_;
	const std::uint32_t hash = text_hash(text, length);
	const std::uint32_t mask = static_cast<std::uint32_t>(index_.size() - 1);
	std::uint32_t position = hash & mask;
	for (AbiNode id = index_[position]; id != ABI_NONE; id = index_[position])
	{
		if (hashes_[id] == hash && nodes_[id].kind == ABI_NODE_STRING)
		{
			const AbiText& existing = texts_[nodes_[id].a];
			if (existing.length == length && std::memcmp(existing.text, text, length) == 0)
			{
				++intern_hits_;
				return id;
			}
		}
		position = (position + 1) & mask;
	}
	AbiNodeData data = make_data(ABI_NODE_STRING);
	data.a = static_cast<std::uint32_t>(texts_.size());
	texts_.push_back(AbiText{ arena_.copy_string(text, length), static_cast<std::uint32_t>(length) });
	const AbiNode id = static_cast<AbiNode>(nodes_.size());
	nodes_.push_back(data);
	hashes_.push_back(hash);
	index_[position] = id;
	if (nodes_.size() * 2 > index_.size())
		grow_index();
	return id;
}

AbiNode AbiGraph::intern(const AbiNodeData& data, const AbiNode* items, std::size_t count)
{
	++intern_requests_;
	if (count != 0 && items >= lists_.data() && items < lists_.data() + lists_.size())
	{
		scratch_.assign(items, items + count);
		items = scratch_.data();
	}
	const std::uint32_t hash = hash_of(data, items, count);
	const std::uint32_t mask = static_cast<std::uint32_t>(index_.size() - 1);
	std::uint32_t position = hash & mask;
	for (AbiNode id = index_[position]; id != ABI_NONE; id = index_[position])
	{
		if (hashes_[id] == hash && same(id, data, items, count))
		{
			++intern_hits_;
			return id;
		}
		position = (position + 1) & mask;
	}
	AbiNodeData stored = data;
	stored.list_begin = static_cast<std::uint32_t>(lists_.size());
	stored.list_size = static_cast<std::uint32_t>(count);
	lists_.insert(lists_.end(), items, items + count);
	const AbiNode id = static_cast<AbiNode>(nodes_.size());
	nodes_.push_back(stored);
	hashes_.push_back(hash);
	index_[position] = id;
	if (nodes_.size() * 2 > index_.size())
		grow_index();
	return id;
}

AbiNode AbiGraph::unary(AbiNodeKind kind, AbiNode operand)
{
	AbiNodeData data = make_data(kind);
	data.a = operand;
	return intern(data, nullptr, 0);
}

namespace {

// Alphabetical order of STRING nodes, then duplicates removed.
void sort_strings(const AbiGraph& graph, std::vector<AbiNode>& strings)
{
	std::sort(strings.begin(), strings.end(), [&graph](AbiNode left, AbiNode right) {
		const AbiText a = graph.text(left);
		const AbiText b = graph.text(right);
		const int order = std::memcmp(a.text, b.text, std::min(a.length, b.length));
		return order != 0 ? order < 0 : a.length < b.length;
	});
	strings.erase(std::unique(strings.begin(), strings.end()), strings.end());
}

} // namespace

AbiNode AbiGraph::std_scope()
{
	return intern(make_data(ABI_NODE_STD), nullptr, 0);
}

AbiNode AbiGraph::function_scope(AbiNode encoding, std::uint32_t default_argument)
{
	AbiNodeData data = make_data(ABI_NODE_FUNCTION_SCOPE);
	data.a = encoding;
	data.b = default_argument;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::data_member_prefix(AbiNode parent, AbiNode member)
{
	AbiNodeData data = make_data(ABI_NODE_NAME);
	data.a = parent;
	data.b = member;
	data.flags = ABI_FLAG_DATA_MEMBER;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::raw_function_scope(AbiNode fragment)
{
	AbiNodeData data = make_data(ABI_NODE_FUNCTION_SCOPE);
	data.flags = ABI_FLAG_RAW;
	data.a = fragment;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::name(AbiNode parent, AbiNode unqualified, const AbiNode* tags, std::size_t tag_count,
	std::uint32_t discriminator, bool internal_linkage)
{
	AbiNodeData data = make_data(ABI_NODE_NAME);
	data.a = parent;
	data.b = unqualified;
	data.c = discriminator;
	data.flags = internal_linkage ? ABI_FLAG_INTERNAL : 0;
	if (tag_count == 0)
		return intern(data, nullptr, 0);
	std::vector<AbiNode> sorted(tags, tags + tag_count);
	sort_strings(*this, sorted);
	return intern(data, sorted.data(), sorted.size());
}

AbiNode AbiGraph::with_tags(AbiNode tagged, const AbiNode* tags, std::size_t tag_count)
{
	const AbiNodeData data = nodes_[tagged];
	if (data.kind == ABI_NODE_TEMPLATE_ID)
	{
		const AbiNode templ = with_tags(data.a, tags, tag_count);
		return template_id(templ, list(tagged), data.list_size);
	}
	if (data.kind != ABI_NODE_NAME && data.kind != ABI_NODE_STD_ABBREVIATION)
		throw InternalError("ABI tags need a named type");
	std::vector<AbiNode> merged(list(tagged), list(tagged) + data.list_size);
	merged.insert(merged.end(), tags, tags + tag_count);
	sort_strings(*this, merged);
	return intern(data, merged.data(), merged.size());
}

AbiNode AbiGraph::tagged_namespace(AbiNode parent, AbiNode unqualified, const AbiNode* tags, std::size_t tag_count)
{
	AbiNodeData data = make_data(ABI_NODE_NAME);
	data.a = parent;
	data.b = unqualified;
	data.flags = ABI_FLAG_HIDDEN_TAGS;
	std::vector<AbiNode> sorted(tags, tags + tag_count);
	sort_strings(*this, sorted);
	return intern(data, sorted.data(), sorted.size());
}

void AbiGraph::sort_tags(std::vector<AbiNode>& tags) const
{
	sort_strings(*this, tags);
}

void AbiGraph::note_implicit_tags(AbiNode derived, AbiNode underived)
{
	if (underived_.find(derived) == nullptr)
		underived_.insert(derived, underived);
}

namespace {

// collect_tags' modes: every tag (plain); every tag but, inside a function
// scope, only written ones (scoped); only written ones (written, below a
// scoped walk's function scope).
const unsigned kPlainTags = 0;
const unsigned kScopedTags = 1;
const unsigned kWrittenTags = 2;
// A node's tag set is memoized whole up to this many tags.
const std::size_t kSmallTagSet = 8;

} // namespace

void AbiGraph::collect_tags(AbiNode node, std::vector<AbiNode>& tags, bool written_in_scopes)
{
	// A node's tags are a function of the interned node and the mode: a
	// small set is memoized per node (a component's set reused when the
	// node adds none), a node with a larger one is walked, each node at
	// most once per query (an epoch marks the visited), its components'
	// small sets taken whole. The result may repeat a tag.
	const unsigned mode = written_in_scopes ? kScopedTags : kPlainTags;
	const std::uint32_t set = tag_set(node, mode);
	if (set >= 3)
	{
		const TagSet found = tag_sets_[set - 3];
		tags.insert(
			tags.end(), tag_set_items_.begin() + found.first, tag_set_items_.begin() + found.first + found.count);
		return;
	}
	if (set != 2)
		return;
	if (++tag_epoch_ == 0)
	{
		std::fill(tag_marks_.begin(), tag_marks_.end(), 0u);
		std::fill(tag_written_marks_.begin(), tag_written_marks_.end(), 0u);
		tag_epoch_ = 1;
	}
	walk_tags(node, tags, mode);
}

std::uint32_t AbiGraph::tag_set(AbiNode node, unsigned mode)
{
	// 1 no tag, 2 more than kSmallTagSet, else 3 + its set's index.
	node = tag_node(node);
	if (!holds_tags(node))
		return 1;
	std::vector<std::uint32_t>& memo = tag_set_memo_[mode];
	if (memo.size() <= node)
		memo.resize(nodes_.size(), 0);
	if (memo[node] != 0)
		return memo[node];
	AbiNode a = ABI_NONE, b = ABI_NONE;
	std::uint32_t count = 0;
	bool own = false;
	tag_components(node, a, b, count, own);
	const AbiNodeData& data = nodes_[node];
	const AbiNode* items = list(node);
	const unsigned below = mode == kWrittenTags || (mode == kScopedTags && data.kind == ABI_NODE_FUNCTION_SCOPE)
		? kWrittenTags
		: mode;
	const bool own_tags = own && data.list_size != 0 &&
		!(mode == kWrittenTags && data.kind == ABI_NODE_NAME && (data.flags & ABI_FLAG_HIDDEN_TAGS) != 0);
	const std::size_t base = tag_scratch_.size();
	if (own_tags)
		tag_scratch_.insert(tag_scratch_.end(), items, items + data.list_size);
	bool large = false;
	std::uint32_t only = 0;
	unsigned contributors = 0;
	const auto add = [&](AbiNode component) {
		const std::uint32_t set = tag_set(component, below);
		if (set == 2)
			large = true;
		else if (set >= 3)
		{
			const TagSet found = tag_sets_[set - 3];
			tag_scratch_.insert(tag_scratch_.end(), tag_set_items_.begin() + found.first,
				tag_set_items_.begin() + found.first + found.count);
			only = set;
			++contributors;
		}
	};
	add(a);
	add(b);
	for (std::uint32_t i = 0; i < count; ++i)
		add(items[i]);
	std::uint32_t result = 2;
	if (!large)
	{
		std::sort(tag_scratch_.begin() + static_cast<std::ptrdiff_t>(base), tag_scratch_.end());
		tag_scratch_.erase(std::unique(tag_scratch_.begin() + static_cast<std::ptrdiff_t>(base), tag_scratch_.end()),
			tag_scratch_.end());
		const std::size_t size = tag_scratch_.size() - base;
		if (size == 0)
			result = 1;
		else if (!own_tags && contributors == 1)
			result = only;
		else if (size <= kSmallTagSet)
		{
			tag_sets_.push_back(
				TagSet{ static_cast<std::uint32_t>(tag_set_items_.size()), static_cast<std::uint32_t>(size) });
			tag_set_items_.insert(
				tag_set_items_.end(), tag_scratch_.begin() + static_cast<std::ptrdiff_t>(base), tag_scratch_.end());
			result = static_cast<std::uint32_t>(tag_sets_.size() + 2);
		}
	}
	tag_scratch_.resize(base);
	memo[node] = result;
	return result;
}

AbiNode AbiGraph::tag_node(AbiNode node)
{
	// An entity nested in another's encoding is walked underived (see
	// note_implicit_tags).
	if (underived_.size() != 0)
	{
		if (const AbiNode* plain = underived_.find(node))
			return *plain;
	}
	return node;
}

void AbiGraph::tag_components(AbiNode node, AbiNode& a, AbiNode& b, std::uint32_t& count, bool& own) const
{
	// The components of a node that can hold ABI tags, and whether its own
	// list is its tags (a name's or an abbreviation's).
	const AbiNodeData& data = nodes_[node];
	bool walk_a = false, walk_b = false, walk_list = false;
	own = false;
	switch (data.kind)
	{
	case ABI_NODE_NAME:
	case ABI_NODE_STD_ABBREVIATION:
		own = true;
		walk_a = walk_b = data.kind == ABI_NODE_NAME;
		break;
	case ABI_NODE_FUNCTION_SCOPE:
		walk_a = (data.flags & ABI_FLAG_RAW) == 0;
		break;
	case ABI_NODE_CONVERSION_NAME:
	case ABI_NODE_CTOR_DTOR_NAME:
	case ABI_NODE_POINTER:
	case ABI_NODE_LVALUE_REFERENCE:
	case ABI_NODE_RVALUE_REFERENCE:
	case ABI_NODE_PACK_EXPANSION:
	case ABI_NODE_COMPLEX:
	case ABI_NODE_QUALIFIED:
	case ABI_NODE_VENDOR_QUALIFIED:
	case ABI_NODE_DECLTYPE:
	case ABI_NODE_LITERAL:
	case ABI_NODE_EXPRESSION_ARGUMENT:
	case ABI_NODE_ENTITY_ARGUMENT:
	case ABI_NODE_SIZEOF_TYPE:
	case ABI_NODE_ENTITY_REFERENCE:
		walk_a = true;
		break;
	case ABI_NODE_ARRAY:
		walk_a = true;
		walk_b = data.flags == ABI_ARRAY_BOUND_EXPRESSION;
		break;
	case ABI_NODE_MEMBER_POINTER:
	case ABI_NODE_CAST:
		walk_a = walk_b = true;
		break;
	case ABI_NODE_ALLOCATION:
		walk_a = walk_b = walk_list = true;
		break;
	case ABI_NODE_FUNCTION_TYPE:
	case ABI_NODE_FUNCTION_ENCODING:
		walk_a = walk_b = walk_list = true;
		break;
	case ABI_NODE_TEMPLATE_ID:
	case ABI_NODE_VENDOR_TYPE:
	case ABI_NODE_TEMPLATE_PARAM_DECL:
	case ABI_NODE_DECLARED_ARGUMENT:
	case ABI_NODE_UNRESOLVED_NAME:
	case ABI_NODE_VENDOR_EXPRESSION:
	case ABI_NODE_SCOPED_MEMBER:
	case ABI_NODE_UNRESOLVED_PREFIX:
	case ABI_NODE_MEMBER_ACCESS:
	case ABI_NODE_EXPRESSION_LIST:
		walk_a = walk_list = true;
		break;
	case ABI_NODE_CLOSURE_NAME:
	case ABI_NODE_ARGUMENT_PACK:
	case ABI_NODE_OPERATION:
	case ABI_NODE_CALL:
		walk_list = true;
		break;
	default:
		break;
	}
	a = walk_a ? data.a : ABI_NONE;
	b = walk_b ? data.b : ABI_NONE;
	count = walk_list ? data.list_size : 0;
}

bool AbiGraph::holds_tags(AbiNode node)
{
	// Whether a node or a component below it carries a tag, once per node.
	node = tag_node(node);
	if (node == ABI_NONE)
		return false;
	if (tag_memo_.size() <= node)
		tag_memo_.resize(nodes_.size(), 0);
	if (tag_memo_[node] != 0)
		return tag_memo_[node] == 2;
	AbiNode a = ABI_NONE, b = ABI_NONE;
	std::uint32_t count = 0;
	bool own = false;
	tag_components(node, a, b, count, own);
	const AbiNode* items = list(node);
	bool holds = own && nodes_[node].list_size != 0;
	holds = holds || holds_tags(a) || holds_tags(b);
	for (std::uint32_t i = 0; i < count && !holds; ++i)
		holds = holds_tags(items[i]);
	tag_memo_[node] = holds ? 2 : 1;
	return holds;
}

void AbiGraph::walk_tags(AbiNode node, std::vector<AbiNode>& tags, unsigned mode)
{
	// Written mode: inside a function scope (Z <encoding> E) of a walk
	// asking for written tags there, only tags the encoding writes count
	// (an inline namespace's do not), as Clang collects a local entity's
	// prefix's tags.
	node = tag_node(node);
	const std::uint32_t set = tag_set(node, mode);
	if (set == 1)
		return;
	if (set >= 3)
	{
		const TagSet found = tag_sets_[set - 3];
		tags.insert(
			tags.end(), tag_set_items_.begin() + found.first, tag_set_items_.begin() + found.first + found.count);
		return;
	}
	const bool written = mode == kWrittenTags;
	std::vector<std::uint32_t>& marks = written ? tag_written_marks_ : tag_marks_;
	if (marks.size() <= node)
		marks.resize(nodes_.size(), 0u);
	if (marks[node] == tag_epoch_ || (written && node < tag_marks_.size() && tag_marks_[node] == tag_epoch_))
		return;
	marks[node] = tag_epoch_;
	AbiNode a = ABI_NONE, b = ABI_NONE;
	std::uint32_t count = 0;
	bool own = false;
	tag_components(node, a, b, count, own);
	const AbiNode* items = list(node);
	const AbiNodeData& data = nodes_[node];
	if (own && !(written && data.kind == ABI_NODE_NAME && (data.flags & ABI_FLAG_HIDDEN_TAGS) != 0))
		tags.insert(tags.end(), items, items + data.list_size);
	const unsigned below = written || (mode == kScopedTags && data.kind == ABI_NODE_FUNCTION_SCOPE) ? kWrittenTags
																									: mode;
	walk_tags(a, tags, below);
	walk_tags(b, tags, below);
	for (std::uint32_t i = 0; i < count; ++i)
		walk_tags(items[i], tags, below);
}

AbiNode AbiGraph::operator_name(AbiTerminalKind fixed_kind)
{
	AbiNodeData data = make_data(ABI_NODE_OPERATOR_NAME);
	data.code = fixed_kind;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::ctor_dtor_name(AbiTerminalKind kind, AbiNode inherited)
{
	AbiNodeData data = make_data(ABI_NODE_CTOR_DTOR_NAME);
	data.code = kind;
	data.a = inherited;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::conversion_name(AbiNode type)
{
	return unary(ABI_NODE_CONVERSION_NAME, type);
}

AbiNode AbiGraph::literal_operator_name(AbiNode suffix)
{
	return unary(ABI_NODE_LITERAL_OPERATOR_NAME, suffix);
}

AbiNode AbiGraph::closure_name(const AbiNode* signature, std::size_t count, std::uint32_t number, bool variadic)
{
	AbiNodeData data = make_data(ABI_NODE_CLOSURE_NAME);
	data.a = number;
	data.flags = variadic ? ABI_FLAG_VARIADIC : 0;
	return intern(data, signature, count);
}

AbiNode AbiGraph::unnamed_type_name(std::uint32_t number)
{
	return unary(ABI_NODE_UNNAMED_TYPE_NAME, number);
}

AbiNode AbiGraph::decomposition_name(const AbiNode* names, std::size_t count)
{
	return intern_with(ABI_NODE_DECOMPOSITION_NAME, ABI_NONE, names, count);
}

AbiNode AbiGraph::template_id(AbiNode templ, const AbiNode* args, std::size_t count)
{
	return intern_with(ABI_NODE_TEMPLATE_ID, templ, args, count);
}

bool AbiGraph::is_char(AbiNode type) const
{
	return nodes_[type].kind == ABI_NODE_BUILTIN && nodes_[type].code == ABI_BUILTIN_TYPE_CHAR;
}

bool AbiGraph::is_std_name(AbiNode name, const char* spelling) const
{
	// A name declared directly in ::std (not in a nested or inline
	// namespace, not local), whatever its ABI tags.
	const AbiNodeData& data = nodes_[name];
	if (data.kind != ABI_NODE_NAME || data.flags != 0 || data.c != 0 || nodes_[data.a].kind != ABI_NODE_STD ||
		nodes_[data.b].kind != ABI_NODE_STRING)
		return false;
	const AbiText written = text(data.b);
	return written.length == std::strlen(spelling) && std::memcmp(written.text, spelling, written.length) == 0;
}

bool AbiGraph::is_char_argument_of(AbiNode type, AbiStandardSubstitutionKind abbreviation) const
{
	// X<char> of the untagged ::std::char_traits (abbreviation NONE) or
	// of the untagged abbreviation.
	const AbiNodeData& data = nodes_[type];
	if (data.kind != ABI_NODE_TEMPLATE_ID || data.list_size != 1 || !is_char(list(type)[0]))
		return false;
	const AbiNodeData& templ = nodes_[data.a];
	if (abbreviation == ABI_STANDARD_SUBSTITUTION_NONE)
		return templ.list_size == 0 && is_std_name(data.a, "char_traits");
	return templ.kind == ABI_NODE_STD_ABBREVIATION && templ.code == abbreviation && templ.list_size == 0;
}

AbiNode AbiGraph::abbreviated_template_id(AbiNode templ, const AbiNode* args, std::size_t count)
{
	// 5.1.10's catalog: ::std::allocator is Sa and ::std::basic_string Sb
	// (template names, whatever the arguments); basic_string<char,
	// char_traits<char>, allocator<char> > is Ss, and basic_istream,
	// basic_ostream and basic_iostream<char, char_traits<char> > are Si, So
	// and Sd (whole types). A name with ABI tags keeps them after its
	// abbreviation (5.1.2), a substitutable component then. Producers that
	// decide abbreviations themselves (PA9's facts) use template_id.
	const bool tagged = nodes_[templ].kind == ABI_NODE_NAME && nodes_[templ].list_size != 0;
	const bool char_traits = count >= 2 && is_char(args[0]) &&
		is_char_argument_of(args[1], ABI_STANDARD_SUBSTITUTION_NONE);
	AbiStandardSubstitutionKind kind = ABI_STANDARD_SUBSTITUTION_NONE;
	bool whole = true;
	if (is_std_name(templ, "allocator"))
	{
		kind = ABI_STANDARD_SUBSTITUTION_ALLOCATOR;
		whole = false;
	}
	else if (is_std_name(templ, "basic_string"))
	{
		whole = count == 3 && char_traits && is_char_argument_of(args[2], ABI_STANDARD_SUBSTITUTION_ALLOCATOR);
		kind = whole ? ABI_STANDARD_SUBSTITUTION_STRING : ABI_STANDARD_SUBSTITUTION_BASIC_STRING;
	}
	else if (count == 2 && char_traits)
	{
		kind = is_std_name(templ, "basic_istream") ? ABI_STANDARD_SUBSTITUTION_ISTREAM
			: is_std_name(templ, "basic_ostream")  ? ABI_STANDARD_SUBSTITUTION_OSTREAM
			: is_std_name(templ, "basic_iostream") ? ABI_STANDARD_SUBSTITUTION_IOSTREAM
												   : ABI_STANDARD_SUBSTITUTION_NONE;
	}
	if (kind == ABI_STANDARD_SUBSTITUTION_NONE)
		return template_id(templ, args, count);
	// The arguments stay valid while the abbreviation is interned (they
	// may be another node's list, which interning can move).
	std::vector<AbiNode> copied;
	if (!whole && count != 0 && args >= lists_.data() && args < lists_.data() + lists_.size())
	{
		copied.assign(args, args + count);
		args = copied.data();
	}
	AbiNode abbreviation = std_abbreviation(kind);
	if (tagged)
		abbreviation = with_tags(abbreviation, list(templ), nodes_[templ].list_size);
	return whole ? abbreviation : template_id(abbreviation, args, count);
}

AbiNode AbiGraph::std_abbreviation(AbiStandardSubstitutionKind kind)
{
	AbiNodeData data = make_data(ABI_NODE_STD_ABBREVIATION);
	data.code = kind;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::builtin(AbiBuiltinTypeKind kind)
{
	AbiNodeData data = make_data(ABI_NODE_BUILTIN);
	data.code = kind;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::vendor_type(AbiNode source_name, const AbiNode* args, std::size_t count)
{
	return intern_with(ABI_NODE_VENDOR_TYPE, source_name, args, count);
}

AbiNode AbiGraph::qualified(AbiNode type, unsigned cv)
{
	if (cv == 0)
		return type;
	if (nodes_[type].kind == ABI_NODE_FUNCTION_TYPE)
	{
		AbiNodeData data = nodes_[type];
		data.code = static_cast<std::uint16_t>(data.code | cv);
		return intern(data, list(type), data.list_size);
	}
	if (nodes_[type].kind == ABI_NODE_QUALIFIED)
	{
		cv |= nodes_[type].code;
		type = nodes_[type].a;
	}
	AbiNodeData data = make_data(ABI_NODE_QUALIFIED);
	data.a = type;
	data.code = static_cast<std::uint16_t>(cv);
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::vendor_qualified(AbiNode type, AbiNode qualifier)
{
	AbiNodeData data = make_data(ABI_NODE_VENDOR_QUALIFIED);
	data.a = type;
	data.b = qualifier;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::array(AbiNode element, AbiArrayBound bound_kind, long long bound_value, AbiNode bound_expression)
{
	AbiNodeData data = make_data(ABI_NODE_ARRAY);
	data.a = element;
	data.flags = bound_kind;
	if (bound_kind == ABI_ARRAY_BOUND_VALUE || bound_kind == ABI_ARRAY_BOUND_VECTOR)
	{
		const unsigned long long bits = static_cast<unsigned long long>(bound_value);
		data.b = static_cast<std::uint32_t>(bits);
		data.c = static_cast<std::uint32_t>(bits >> 32);
	}
	else if (bound_kind == ABI_ARRAY_BOUND_EXPRESSION)
		data.b = bound_expression;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::function_type(
	AbiNode result, const AbiNode* params, std::size_t count, bool variadic, unsigned qualifiers)
{
	AbiNodeData data = make_data(ABI_NODE_FUNCTION_TYPE);
	data.a = result;
	data.flags = variadic ? ABI_FLAG_VARIADIC : 0;
	data.code = static_cast<std::uint16_t>(qualifiers);
	return intern(data, params, count);
}

AbiNode AbiGraph::member_pointer(AbiNode class_type, AbiNode member_type)
{
	if (nodes_[member_type].kind == ABI_NODE_FUNCTION_TYPE && nodes_[member_type].b != class_type)
	{
		AbiNodeData method = nodes_[member_type];
		method.b = class_type;
		member_type = intern(method, list(member_type), method.list_size);
	}
	AbiNodeData data = make_data(ABI_NODE_MEMBER_POINTER);
	data.a = class_type;
	data.b = member_type;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::template_param(std::uint32_t index, bool substitutable, std::uint32_t level, std::uint32_t kind)
{
	AbiNodeData data = make_data(ABI_NODE_TEMPLATE_PARAM);
	data.a = index;
	data.b = level;
	data.c = kind;
	data.flags = substitutable ? ABI_FLAG_SUBSTITUTABLE : 0;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::decltype_of(AbiNode expression, bool id_expression)
{
	AbiNodeData data = make_data(ABI_NODE_DECLTYPE);
	data.a = expression;
	data.flags = id_expression ? ABI_FLAG_ID_EXPRESSION : 0;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::literal(AbiNode type, long long value, bool unsigned_value)
{
	AbiNodeData data = make_data(ABI_NODE_LITERAL);
	const unsigned long long bits = static_cast<unsigned long long>(value);
	data.flags = unsigned_value ? ABI_FLAG_UNSIGNED_VALUE : 0;
	data.a = type;
	data.b = static_cast<std::uint32_t>(bits);
	data.c = static_cast<std::uint32_t>(bits >> 32);
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::nullptr_literal()
{
	AbiNodeData data = make_data(ABI_NODE_LITERAL);
	data.a = builtin(ABI_BUILTIN_TYPE_NULLPTR);
	data.flags = ABI_FLAG_NO_VALUE;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::template_param_decl(
	AbiTemplateParamDeclKind kind, bool pack, AbiNode type, const AbiNode* head, std::size_t count)
{
	AbiNodeData data = make_data(ABI_NODE_TEMPLATE_PARAM_DECL);
	data.code = static_cast<std::uint16_t>(kind);
	data.flags = pack ? ABI_FLAG_PACK : 0;
	data.a = type;
	return intern(data, head, count);
}

AbiNode AbiGraph::declared_argument(const AbiNode* declarations, std::size_t count, AbiNode argument)
{
	AbiNodeData data = make_data(ABI_NODE_DECLARED_ARGUMENT);
	data.a = argument;
	return intern(data, declarations, count);
}

AbiNode AbiGraph::typed_argument(AbiNode parameter_type, AbiNode argument)
{
	const AbiNode declaration = template_param_decl(ABI_PARAM_DECL_NON_TYPE, false, parameter_type);
	return declared_argument(&declaration, 1, argument);
}

AbiNode AbiGraph::expression_argument(AbiNode expression)
{
	return unary(ABI_NODE_EXPRESSION_ARGUMENT, expression);
}

AbiNode AbiGraph::argument_pack(const AbiNode* args, std::size_t count)
{
	return intern(make_data(ABI_NODE_ARGUMENT_PACK), args, count);
}

AbiNode AbiGraph::entity_argument(AbiNode entity, bool address_of)
{
	AbiNodeData data = make_data(ABI_NODE_ENTITY_ARGUMENT);
	data.a = entity;
	data.flags = address_of ? ABI_FLAG_ADDRESS_OF : 0;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::function_param(std::uint32_t index, std::uint32_t level)
{
	AbiNodeData data = make_data(ABI_NODE_FUNCTION_PARAM);
	data.a = index;
	data.b = level;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::this_param()
{
	AbiNodeData data = make_data(ABI_NODE_FUNCTION_PARAM);
	data.flags = ABI_FLAG_THIS;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::operation(std::uint16_t code, const AbiNode* operands, std::size_t count, bool prefix)
{
	AbiNodeData data = make_data(ABI_NODE_OPERATION);
	data.code = code;
	data.flags = prefix ? ABI_FLAG_PREFIX : 0;
	return intern(data, operands, count);
}

AbiNode AbiGraph::call(const AbiNode* callee_and_args, std::size_t count, bool parenthesized)
{
	AbiNodeData data = make_data(ABI_NODE_CALL);
	data.flags = parenthesized ? ABI_FLAG_PARENTHESIZED : 0;
	return intern(data, callee_and_args, count);
}

AbiNode AbiGraph::cast(std::uint16_t code, AbiNode type, AbiNode operand)
{
	AbiNodeData data = make_data(ABI_NODE_CAST);
	data.code = code;
	data.a = type;
	data.b = operand;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::unresolved_name(AbiNode source_name, const AbiNode* args, std::size_t count)
{
	return intern_with(ABI_NODE_UNRESOLVED_NAME, source_name, args, count);
}

AbiNode AbiGraph::vendor_expression(AbiNode source_name, const AbiNode* args, std::size_t count)
{
	return intern_with(ABI_NODE_VENDOR_EXPRESSION, source_name, args, count);
}

AbiNode AbiGraph::sizeof_type(AbiNode type)
{
	return unary(ABI_NODE_SIZEOF_TYPE, type);
}

AbiNode AbiGraph::type_operation(std::uint16_t code, AbiNode type)
{
	AbiNodeData data = make_data(ABI_NODE_SIZEOF_TYPE);
	data.code = code == abi_pack_operation_code('s', 't') ? 0 : code;
	data.a = type;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::scoped_member(
	AbiNode qualifier_type, AbiNode member, bool close_qualifier, const AbiNode* args, std::size_t count)
{
	AbiNodeData data = make_data(ABI_NODE_SCOPED_MEMBER);
	data.a = qualifier_type;
	data.b = member;
	data.flags = close_qualifier ? ABI_FLAG_CLOSE_QUALIFIER : 0;
	return intern(data, args, count);
}

AbiNode AbiGraph::unresolved_prefix(AbiNode unresolved_type, const AbiNode* levels, std::size_t count, bool global)
{
	AbiNodeData data = make_data(ABI_NODE_UNRESOLVED_PREFIX);
	data.a = unresolved_type;
	data.flags = global ? ABI_FLAG_GLOBAL : 0;
	return intern(data, levels, count);
}

AbiNode AbiGraph::member_access(
	std::uint16_t code, AbiNode object, AbiNode member, const AbiNode* args, std::size_t count)
{
	AbiNodeData data = make_data(ABI_NODE_MEMBER_ACCESS);
	data.code = code;
	data.a = object;
	data.b = member;
	return intern(data, args, count);
}

AbiNode AbiGraph::entity_reference(AbiNode entity)
{
	return unary(ABI_NODE_ENTITY_REFERENCE, entity);
}

AbiNode AbiGraph::expression_list(std::uint16_t code, AbiNode type, const AbiNode* expressions, std::size_t count)
{
	AbiNodeData data = make_data(ABI_NODE_EXPRESSION_LIST);
	data.code = code;
	data.a = type;
	return intern(data, expressions, count);
}

AbiNode AbiGraph::allocation(
	std::uint16_t code, bool global, AbiNode type, const AbiNode* operands, std::size_t count, AbiNode initializer)
{
	AbiNodeData data = make_data(ABI_NODE_ALLOCATION);
	data.code = code;
	data.flags = global ? ABI_FLAG_GLOBAL : 0;
	data.a = type;
	data.b = initializer;
	return intern(data, operands, count);
}

AbiNode AbiGraph::floating_literal(AbiNode type, unsigned long long bits)
{
	AbiNodeData data = make_data(ABI_NODE_LITERAL);
	data.a = type;
	data.b = static_cast<std::uint32_t>(bits);
	data.c = static_cast<std::uint32_t>(bits >> 32);
	data.flags = ABI_FLAG_FLOATING;
	return intern(data, nullptr, 0);
}

AbiNode AbiGraph::function_encoding(
	AbiNode name, AbiNode result, const AbiNode* params, std::size_t count, bool variadic, unsigned qualifiers)
{
	AbiNodeData data = make_data(ABI_NODE_FUNCTION_ENCODING);
	data.a = name;
	data.b = result;
	data.flags = variadic ? ABI_FLAG_VARIADIC : 0;
	data.code = static_cast<std::uint16_t>(qualifiers);
	return intern(data, params, count);
}

AbiNode AbiGraph::external_symbol(AbiNode mangled_name)
{
	return unary(ABI_NODE_EXTERNAL_SYMBOL, mangled_name);
}

AbiNode AbiGraph::intern_with(AbiNodeKind kind, AbiNode a, const AbiNode* items, std::size_t count)
{
	AbiNodeData data = make_data(kind);
	data.a = a;
	return intern(data, items, count);
}

} // namespace abi_mangle
