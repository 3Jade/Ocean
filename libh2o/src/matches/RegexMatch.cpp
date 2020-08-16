#include "RegexMatch.hpp"
#include "../items/LiteralItem.hpp"
#include <sprawl/memory/PoolAllocator.hpp>

RegexMatch::RegexMatch(LiteralItem& baseItem, TokenList&& tokens, re2::StringPiece* stringPieces)
	: Match(baseItem, std::move(tokens))
	, m_stringPieces(stringPieces)
{
	// NOP
}

RegexMatch::~RegexMatch()
{
	delete[] m_stringPieces;
}

std::string_view RegexMatch::Group(size_t index) const
{
	return std::string_view(m_stringPieces[index].data(), m_stringPieces[index].length());
}

std::string_view RegexMatch::Group(std::string_view const& groupName) const
{
	LiteralItem& item = static_cast<LiteralItem&>(GetBaseItem());
	return Group(item.GroupNameToIndex(groupName));
}

bool RegexMatch::GetLiteralValue(BytecodeWriter& writer, OceanValue& outValue) const
{
	LiteralItem& item = static_cast<LiteralItem&>(GetBaseItem());
	return item.GetLiteralValue(writer, *this, outValue);
}

typedef sprawl::memory::DynamicPoolAllocator<sizeof(RegexMatch)> matchAllocator;

RegexMatch* RegexMatch::Create(LiteralItem& baseItem, TokenList&& tokens, re2::StringPiece* stringPieces)
{
	RegexMatch* ret = (RegexMatch*)matchAllocator::alloc();
	new(ret) RegexMatch(baseItem, std::move(tokens), stringPieces);
	return ret;
}

void RegexMatch::Release()
{
	this->~RegexMatch();
	matchAllocator::free(this);
}
