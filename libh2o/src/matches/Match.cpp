#include "Match.hpp"

Match::Match(Item& baseItem, TokenList&& tokens)
	: m_baseItem(baseItem)
	, m_tokens(std::move(tokens))
{
	// NOP
}

size_t Match::End() const
{
	return m_tokens.Length();
}

Item& Match::GetBaseItem() const
{
	return m_baseItem;
}

const TokenList& Match::Tokens()
{
	return m_tokens;
}

bool Match::Translate(BytecodeWriter& writer) const
{
	return m_baseItem.Translate(writer, *this);
}
