#include "StringMatch.hpp"
#include "../items/RegexSpanItem.hpp"

StringMatch::StringMatch(Item& baseItem, TokenList&& tokens, sprawl::StringRef const& stringData)
	: Match(baseItem, std::move(tokens))
	, m_stringData(stringData)
{
	// NOP
}

sprawl::StringRef StringMatch::GetString() const
{
	return m_stringData;
}


typedef sprawl::memory::DynamicPoolAllocator<sizeof(StringMatch)> matchAllocator;

StringMatch* StringMatch::Create(Item& baseItem, TokenList&& tokens, const sprawl::StringRef& stringData)
{
	StringMatch* ret = (StringMatch*)matchAllocator::alloc();
	new(ret) StringMatch(baseItem, std::move(tokens), stringData);
	return ret;
}

void StringMatch::Release()
{
	this->~StringMatch();
	matchAllocator::free(this);
}
