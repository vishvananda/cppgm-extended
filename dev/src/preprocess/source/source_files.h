#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "preprocess/source/source_buffer.h"
#include "support/telemetry.h"

struct Identifier;

// A file's host identity: device and inode numbers from `stat`, as the
// course scaffold's GetPreprocessorFileId reports them. Distinct path
// spellings of one file share it.
struct FileIdentity
{
	unsigned long long device;
	unsigned long long inode;

	bool operator==(const FileIdentity& other) const { return device == other.device && inode == other.inode; }
};

struct FileIdentityHash
{
	std::size_t operator()(const FileIdentity& identity) const
	{
		return static_cast<std::size_t>(identity.inode * 0x9E3779B97F4A7C15ull ^ identity.device);
	}
};

// The identity of the regular file at `path`; false if there is none.
bool GetFileIdentity(const std::string& path, FileIdentity& identity);

// One physical file read by a translation unit, with its immutable image.
class SourceFile
{
public:
	SourceFile(std::string path, const FileIdentity& identity, bool has_identity, SourceBuffer buffer)
		: path_(std::move(path)), identity_(identity), has_identity_(has_identity), buffer_(std::move(buffer))
	{}

	// The path the file was first opened by.
	const std::string& path() const { return path_; }
	const SourceBuffer& buffer() const { return buffer_; }
	bool has_identity() const { return has_identity_; }
	const FileIdentity& identity() const { return identity_; }

	// The 1-based physical line of a buffer offset. The table of line starts
	// is built on the first query, in one pass over the image.
	uint32_t physical_line(uint32_t offset);
	// The 1-based byte column of a buffer offset on its physical line (the
	// first line starts after any byte order mark).
	uint32_t physical_column(uint32_t offset);

	// The macro guarding the whole file, found when the file was read: the
	// file is `#ifndef guard` ... `#endif` with nothing else outside and no
	// #elif or #else for that #ifndef, so while the macro is defined an
	// inclusion produces nothing (the multiple-include optimization).
	const Identifier* guard() const { return guard_; }
	void set_guard(const Identifier* guard) { guard_ = guard; }

private:
	std::string path_;
	FileIdentity identity_;
	bool has_identity_;
	SourceBuffer buffer_;
	std::vector<uint32_t> line_starts_;
	const Identifier* guard_ = nullptr;
};

// The presumed name and line delta line control left in force up to (not
// including) a location: the states before an inclusion's latest #line.
struct SourceLineState
{
	uint32_t until;
	int64_t line_delta;
	std::string presumed_name;
};

// No inclusion, and no location: the parent and the `included_at` of an
// inclusion no #include entered (the primary source file, and the
// predefined, command-line and built-in texts read as if at its start).
const uint32_t kNoInclusion = 0xffffffffu;
const uint32_t kNoLocation = 0xffffffffu;

// One inclusion of a file: its locations are [base, base + size]. Line
// control ([cpp.line] 16.4) changes its presumed name, which is __FILE__
// and the base of relative #include paths, and its presumed line numbers:
// physical line n is presumed line n + line_delta. The state stays with the
// inclusion, so a token keeps naming its own file after the file ends; the
// states earlier #line directives set stay too (`earlier`, in location
// order), so a later phase reads each location's own. An #include's
// inclusion records its includer and the directive's location, from which
// a diagnostic names the chain of files that included its location.
struct SourceInclusion
{
	uint32_t base;
	// The including inclusion (kNoInclusion for none); included_at below
	// is the location of the #include directive in it.
	uint32_t parent;
	SourceFile* file;
	std::string presumed_name;
	int64_t line_delta;
	// __FILE__'s spelling for presumed_name, made when first needed (the
	// spelling is owned by the translation unit's arena).
	const char* file_literal;
	uint32_t file_literal_length;
	uint32_t included_at;
	std::vector<SourceLineState> earlier;
	// A system header (GCC's and Clang's notion): found in a system
	// directory (hosted mode's -isystem and host directories), or beside
	// the system header that included it.
	bool system;
};

// The files of one translation unit and its location space. Each file is
// read once and shared by all its inclusions; each inclusion reserves a
// range of 32-bit locations, so a location names a file inclusion and an
// offset without any per-token file field.
class SourceFiles
{
public:
	SourceFiles();
	~SourceFiles();

	// The file at `path` if it names a readable regular file, else null.
	// Each path is looked up once per translation unit; files are assumed
	// not to change while it is preprocessed.
	SourceFile* open(const std::string& path);

	// A file that is not on disk, such as the predefined macro definitions.
	SourceFile* add_text(std::string name, std::string text);

	// Reserves the locations of one inclusion of `file`, included by the
	// directive at `included_at` in inclusion `parent` (kNoInclusion: none),
	// and returns its index.
	uint32_t enter(SourceFile* file, std::string presumed_name, uint32_t parent, uint32_t included_at);

	SourceInclusion& inclusion(uint32_t index) { return inclusions_[index]; }
	const SourceInclusion& inclusion(uint32_t index) const { return inclusions_[index]; }
	// The inclusion containing a location.
	SourceInclusion& inclusion_at(uint32_t location);
	const SourceInclusion& inclusion_at(uint32_t location) const;

	uint32_t file_count() const { return static_cast<uint32_t>(files_.size()); }
	uint32_t inclusion_count() const { return static_cast<uint32_t>(inclusions_.size()); }
	// Bytes of source images held (telemetry).
	telemetry::Counter bytes_read() const { return bytes_read_; }

private:
	SourceFile* find_or_read(const std::string& path);

	std::vector<std::unique_ptr<SourceFile>> files_;
	std::unordered_map<FileIdentity, SourceFile*, FileIdentityHash> by_identity_;
	// Paths looked up so far, null for those that name no file.
	std::unordered_map<std::string, SourceFile*> by_path_;
	std::vector<SourceInclusion> inclusions_;
	uint32_t next_base_;
	telemetry::Counter bytes_read_;
};
