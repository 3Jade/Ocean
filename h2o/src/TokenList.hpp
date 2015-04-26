#pragma once

#include "Token.hpp"

class TokenList
{
public:
	TokenList();
	TokenList(Token* existingArray, size_t length);
	~TokenList();

	TokenList(TokenList&& other);

	void PushBack(Token const& token);
	TokenList Slice(size_t start, int end) const;
	size_t Length() const;

	Token const& operator[](size_t index) const;
private:
	Token* m_tokenArray;
	size_t m_length;
	size_t m_capacity;
	bool m_allocated;
};

inline size_t TokenList::Length() const
{
	return m_length;
}

inline Token const& TokenList::operator[](size_t index) const
{
	return m_tokenArray[index];
}
