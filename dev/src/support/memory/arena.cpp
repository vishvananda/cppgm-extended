#include "support/memory/arena.h"

#include <cstdlib>

namespace {

const std::size_t kFirstBlockSize = 16 * 1024;
const std::size_t kMaxBlockGrowth = 8 * 1024 * 1024;

} // namespace

Arena::~Arena()
{
	while (head_ != nullptr)
	{
		Block* next = head_->next;
		std::free(head_);
		head_ = next;
	}
}

void* Arena::allocate_slow(std::size_t size, std::size_t alignment)
{
	// Blocks double up to a limit, so the number of blocks is logarithmic in
	// the total size and the unused tail of each block stays bounded.
	std::size_t block_size = bytes_reserved_ == 0 ? kFirstBlockSize : bytes_reserved_;
	if (block_size > kMaxBlockGrowth)
		block_size = kMaxBlockGrowth;
	const std::size_t needed = sizeof(Block) + size + alignment;
	if (block_size < needed)
		block_size = needed;

	Block* block = static_cast<Block*>(std::malloc(block_size));
	if (block == nullptr)
		throw std::bad_alloc();
	block->next = head_;
	block->size = block_size;
	head_ = block;
	bytes_reserved_ += block_size;
	next_ = reinterpret_cast<char*>(block + 1);
	limit_ = reinterpret_cast<char*>(block) + block_size;
	return allocate(size, alignment);
}

void Arena::rewind(const Mark& mark)
{
	while (head_ != mark.head)
	{
		Block* next = head_->next;
		bytes_reserved_ -= head_->size;
		std::free(head_);
		head_ = next;
	}
	next_ = mark.next;
	limit_ = mark.limit;
}
