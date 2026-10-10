#include "preprocess/source/source_files.h"

#include <sys/stat.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>

#include "preprocess/preprocess_error.h"

bool GetFileIdentity(const std::string& path, FileIdentity& identity)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode))
		return false;
	identity.device = static_cast<unsigned long long>(info.st_dev);
	identity.inode = static_cast<unsigned long long>(info.st_ino);
	return true;
}

uint32_t SourceFile::physical_line(uint32_t offset)
{
	if (line_starts_.empty())
	{
		const char* data = buffer_.data();
		const uint32_t size = buffer_.size();
		line_starts_.push_back(buffer_.begin_offset());
		for (uint32_t at = buffer_.begin_offset(); at < size;)
		{
			const void* found = std::memchr(data + at, '\n', size - at);
			if (found == nullptr)
				break;
			at = static_cast<uint32_t>(static_cast<const char*>(found) - data) + 1;
			line_starts_.push_back(at);
		}
	}
	// The number of line starts at or before the offset.
	return static_cast<uint32_t>(
		std::upper_bound(line_starts_.begin(), line_starts_.end(), offset) - line_starts_.begin());
}

uint32_t SourceFile::physical_column(uint32_t offset)
{
	const uint32_t line = physical_line(offset);
	const uint32_t start = line_starts_[line == 0 ? 0 : line - 1];
	return offset > start ? offset - start + 1 : 1;
}

SourceFiles::SourceFiles() : next_base_(0), bytes_read_(0)
{}

SourceFiles::~SourceFiles() = default;

SourceFile* SourceFiles::open(const std::string& path)
{
	auto cached = by_path_.find(path);
	if (cached != by_path_.end())
		return cached->second;
	SourceFile* file = find_or_read(path);
	by_path_.emplace(path, file);
	return file;
}

SourceFile* SourceFiles::find_or_read(const std::string& path)
{
	FileIdentity identity;
	if (!GetFileIdentity(path, identity))
		return nullptr;
	auto found = by_identity_.find(identity);
	if (found != by_identity_.end())
		return found->second;
	std::ifstream in(path, std::ios::binary);
	if (!in)
		return nullptr;
	files_.emplace_back(new SourceFile(path, identity, true, SourceBuffer::ReadFrom(in)));
	bytes_read_ += files_.back()->buffer().size();
	by_identity_.emplace(identity, files_.back().get());
	return files_.back().get();
}

SourceFile* SourceFiles::add_text(std::string name, std::string text)
{
	files_.emplace_back(new SourceFile(std::move(name), FileIdentity{ 0, 0 }, false, SourceBuffer(std::move(text))));
	bytes_read_ += files_.back()->buffer().size();
	return files_.back().get();
}

uint32_t SourceFiles::enter(SourceFile* file, std::string presumed_name, uint32_t parent, uint32_t included_at)
{
	// One location past the image names its end, and the next inclusion
	// starts after it.
	const uint64_t end = uint64_t(next_base_) + file->buffer().size() + 1;
	if (end > std::numeric_limits<uint32_t>::max())
		throw PreprocessError(0, "the translation unit exceeds the source location space");
	inclusions_.push_back(
		SourceInclusion{ next_base_, parent, file, std::move(presumed_name), 0, nullptr, 0, included_at });
	next_base_ = static_cast<uint32_t>(end);
	return static_cast<uint32_t>(inclusions_.size() - 1);
}

SourceInclusion& SourceFiles::inclusion_at(uint32_t location)
{
	return const_cast<SourceInclusion&>(static_cast<const SourceFiles*>(this)->inclusion_at(location));
}

const SourceInclusion& SourceFiles::inclusion_at(uint32_t location) const
{
	// Inclusions are recorded in increasing base order.
	auto after = std::upper_bound(inclusions_.begin(), inclusions_.end(), location,
		[](uint32_t value, const SourceInclusion& inclusion) { return value < inclusion.base; });
	return *(after - 1);
}
