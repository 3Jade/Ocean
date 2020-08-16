#pragma once

#include <string_view>

class StringList
{
public:
	StringList();
	StringList(std::string_view* existingArray, size_t length);
	~StringList();

	StringList(StringList&& other) noexcept;
	StringList(StringList const& other);

	void PushBack(std::string_view const& string);
	StringList Slice(size_t start, int end=-1);
	size_t Length() const;

	std::string_view const& operator[](size_t index) const;
private:
	std::string_view* m_stringArray;
	size_t m_length;
	size_t m_capacity;
	bool m_allocated;
};

inline size_t StringList::Length() const
{
	return m_length;
}

inline std::string_view const& StringList::operator[](size_t index) const
{
	return m_stringArray[index];
}
