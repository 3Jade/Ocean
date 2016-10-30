#pragma once

#include "../items/Item.hpp"

class BytecodeWriter;

enum class MatchType
{
	Expression,
	Regex,
	String
};

class Match
{
public:
	Match(Item& baseItem, TokenList&& tokens);
	size_t End() const;
	Item& GetBaseItem() const;
	TokenList const& Tokens();

	bool Translate(BytecodeWriter& writer) const;

	virtual MatchType GetType() const = 0;

	virtual void Print() const = 0;

	virtual void Release() = 0;
private:
	Item& m_baseItem;
	TokenList m_tokens;
};
