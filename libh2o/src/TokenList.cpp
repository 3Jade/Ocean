#include "TokenList.hpp"

TokenList::TokenList()
	: m_tokenArray((Token*)malloc(sizeof(Token)*256))
	, m_length(0)
	, m_capacity(256)
	, m_allocated(true)
{
	// NOP
}

TokenList::TokenList(Token* existingArray, size_t length)
	: m_tokenArray(existingArray)
	, m_length(length)
	, m_capacity(0)
	, m_allocated(false)
{
	// NOP
}

TokenList::~TokenList()
{
	if(m_allocated)
	{
		free(m_tokenArray);
	}
}

TokenList::TokenList(TokenList&& other)
	: m_tokenArray(other.m_tokenArray)
	, m_length(other.m_length)
	, m_capacity(other.m_capacity)
	, m_allocated(other.m_allocated)
{
	other.m_tokenArray = nullptr;
	other.m_length = 0;
	other.m_capacity = 0;
	other.m_allocated = false;
}

void TokenList::PushBack(Token const& token)
{
	if(m_length > m_capacity * 0.75)
	{
		m_capacity = m_capacity * 2;
		Token* newArray = (Token*)malloc(sizeof(Token) * m_capacity);
		for(int i = 0; i < m_length; ++i)
		{
			newArray[i] = m_tokenArray[i];
		}
		free(m_tokenArray);
		m_tokenArray = newArray;
	}
	m_tokenArray[m_length++] = token;
}

TokenList TokenList::Slice(size_t start, int end) const
{
	if(end < 0)
	{
		end = m_length - end - 1;
	}
	return TokenList(m_tokenArray+start, end-start);
}
