#include "Match.hpp"

Match::Match(Item& baseItem, size_t endIndex)
: m_baseItem(baseItem)
, m_endIndex(endIndex)
{
	// NOP
}

size_t Match::End() const
{
	return m_endIndex;
}

Item& Match::GetBaseItem() const
{
	return m_baseItem;
}

bool Match::Translate(BytecodeWriter& writer) const
{
	return m_baseItem.Translate(writer, *this);
}
