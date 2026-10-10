#pragma once

#include <cstdio>

// An open stdio stream owned by a scope: closed when the scope ends, so an
// error leaving it cannot leak the stream. close() closes it early and
// reports whether buffered output reached the file.
class FileHandle
{
public:
	explicit FileHandle(std::FILE* file = nullptr) : file_(file) {}
	~FileHandle()
	{
		if (file_ != nullptr)
			std::fclose(file_);
	}
	FileHandle(const FileHandle&) = delete;
	FileHandle& operator=(const FileHandle&) = delete;

	std::FILE* get() const { return file_; }

	// False when closing the stream failed (its output may be lost).
	bool close()
	{
		std::FILE* const file = file_;
		file_ = nullptr;
		return file == nullptr || std::fclose(file) == 0;
	}

private:
	std::FILE* file_;
};
