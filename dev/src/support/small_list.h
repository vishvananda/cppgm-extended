#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

// A short list of trivially copyable values under construction: the first
// N live in the list itself and only a longer list takes heap storage, so
// the lists analysis builds per use (a template-id's arguments, a
// deduction's results) cost no allocation in the common case.
template<typename T, uint32_t N>
class SmallList
{
	static_assert(std::is_trivially_copyable<T>::value, "small lists copy bitwise");

public:
	SmallList() : data_(inline_), size_(0), capacity_(N) {}
	SmallList(size_t count, const T& value) : SmallList() { resize(count, value); }
	SmallList(const SmallList& other) : SmallList() { assign(other.data_, other.size_); }
	~SmallList()
	{
		if (data_ != inline_)
			::operator delete(data_);
	}

	SmallList& operator=(const SmallList& other)
	{
		if (this != &other)
			assign(other.data_, other.size_);
		return *this;
	}

	size_t size() const { return size_; }
	bool empty() const { return size_ == 0; }
	T* data() { return data_; }
	const T* data() const { return data_; }
	T& operator[](size_t index) { return data_[index]; }
	const T& operator[](size_t index) const { return data_[index]; }
	T* begin() { return data_; }
	T* end() { return data_ + size_; }
	const T* begin() const { return data_; }
	const T* end() const { return data_ + size_; }

	T& back() { return data_[size_ - 1]; }
	void pop_back() { --size_; }
	void clear() { size_ = 0; }

	void push_back(const T& value)
	{
		if (size_ == capacity_)
			reserve(size_ + 1);
		data_[size_++] = value;
	}

	void resize(size_t count, const T& value = T())
	{
		reserve(count);
		for (size_t i = size_; i < count; ++i)
			data_[i] = value;
		size_ = static_cast<uint32_t>(count);
	}

	void assign(const T* values, size_t count)
	{
		size_ = 0;
		reserve(count);
		if (count != 0)
			std::memcpy(static_cast<void*>(data_), values, sizeof(T) * count);
		size_ = static_cast<uint32_t>(count);
	}

	bool operator==(const SmallList& other) const
	{
		return size_ == other.size_ && (size_ == 0 || std::memcmp(data_, other.data_, sizeof(T) * size_) == 0);
	}
	bool operator!=(const SmallList& other) const { return !(*this == other); }

private:
	void reserve(size_t count)
	{
		if (count <= capacity_)
			return;
		size_t capacity = capacity_ * 2;
		if (capacity < count)
			capacity = count;
		T* grown = static_cast<T*>(::operator new(sizeof(T) * capacity));
		if (size_ != 0)
			std::memcpy(static_cast<void*>(grown), data_, sizeof(T) * size_);
		if (data_ != inline_)
			::operator delete(data_);
		data_ = grown;
		capacity_ = static_cast<uint32_t>(capacity);
	}

	T* data_;
	uint32_t size_;
	uint32_t capacity_;
	T inline_[N];
};
