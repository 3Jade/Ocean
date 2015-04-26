#include "RegexMatch.hpp"
#include "../items/LiteralItem.hpp"

RegexMatch::RegexMatch(LiteralItem& baseItem, size_t endIndex, re2::StringPiece* stringPieces)
	: Match(baseItem, endIndex)
	, m_stringPieces(stringPieces)
{
	// NOP
}

RegexMatch::~RegexMatch()
{
	delete[] m_stringPieces;
}

sprawl::String RegexMatch::Group(size_t index) const
{
	return sprawl::StringLiteral(m_stringPieces[index].data(), m_stringPieces[index].length());
}

sprawl::String RegexMatch::Group(sprawl::String const& groupName) const
{
	LiteralItem& item = static_cast<LiteralItem&>(GetBaseItem());
	return Group(item.GroupNameToIndex(groupName));
}

bool RegexMatch::GetLiteralValue(BytecodeWriter& writer, int64_t& outValue) const
{
	LiteralItem& item = static_cast<LiteralItem&>(GetBaseItem());
	return item.GetLiteralValue(writer, *this, outValue);
}

typedef sprawl::memory::DynamicPoolAllocator<sizeof(RegexMatch)> matchAllocator;

RegexMatch* RegexMatch::Create(LiteralItem& baseItem, size_t endIndex, re2::StringPiece* stringPieces)
{
	RegexMatch* ret = (RegexMatch*)matchAllocator::alloc();
	new(ret) RegexMatch(baseItem, endIndex, stringPieces);
	return ret;
}

void RegexMatch::Release()
{
	this->~RegexMatch();
	matchAllocator::free(this);
}
