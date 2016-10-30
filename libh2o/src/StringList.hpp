#pragma once

#include "sprawl/string/String.hpp"

class StringList
{
public:
	StringList();
	StringList(sprawl::StringLiteral* existingArray, size_t length);
	~StringList();

	StringList(StringList&& other);
	StringList(StringList const& other);

	void PushBack(sprawl::StringLiteral const& string);
	StringList Slice(size_t start, int end=-1);
	size_t Length() const;

	sprawl::StringLiteral const& operator[](size_t index) const;
private:
	sprawl::StringLiteral* m_stringArray;
	size_t m_length;
	size_t m_capacity;
	bool m_allocated;
};

inline size_t StringList::Length() const
{
	return m_length;
}

inline sprawl::StringLiteral const& StringList::operator[](size_t index) const
{
	return m_stringArray[index];
}
