#include "StringList.hpp"

StringList::StringList()
	: m_stringArray((sprawl::StringLiteral*)malloc(sizeof(sprawl::StringLiteral)*256))
	, m_length(0)
	, m_capacity(256)
	, m_allocated(true)
{
	// NOP
}

StringList::StringList(sprawl::StringLiteral* existingArray, size_t length)
	: m_stringArray(existingArray)
	, m_length(length)
	, m_capacity(0)
	, m_allocated(false)
{
	// NOP
}

StringList::~StringList()
{
	if(m_allocated)
	{
		free(m_stringArray);
	}
}

StringList::StringList(StringList&& other)
	: m_stringArray(other.m_stringArray)
	, m_length(other.m_length)
	, m_capacity(other.m_capacity)
	, m_allocated(other.m_allocated)
{
	other.m_stringArray = nullptr;
	other.m_length = 0;
	other.m_capacity = 0;
	other.m_allocated = false;
}

StringList::StringList(StringList const& other)
	: m_stringArray(nullptr)
	, m_length(other.m_length)
	, m_capacity(other.m_capacity)
	, m_allocated(true)
{
	m_stringArray = (sprawl::StringLiteral*)malloc(sizeof(sprawl::StringLiteral) * m_capacity);
	for(int i = 0; i < m_length; ++i)
	{
		m_stringArray[i] = other.m_stringArray[i];
	}
}

void StringList::PushBack(sprawl::StringLiteral const& string)
{
	if(m_length > m_capacity * 0.75)
	{
		m_capacity = m_capacity * 2;
		sprawl::StringLiteral* newArray = (sprawl::StringLiteral*)malloc(sizeof(sprawl::StringLiteral) * m_capacity);
		for(int i = 0; i < m_length; ++i)
		{
			newArray[i] = m_stringArray[i];
		}
		free(m_stringArray);
		m_stringArray = newArray;
	}
	m_stringArray[m_length++] = string;
}

StringList StringList::Slice(size_t start, int end)
{
	if(end < 0)
	{
		end = m_length - end - 1;
	}
	return StringList(m_stringArray+start, end-start);
}
