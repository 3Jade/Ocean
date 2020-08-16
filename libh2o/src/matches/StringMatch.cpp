#include "StringMatch.hpp"
#include "../items/RegexSpanItem.hpp"
#include <sprawl/memory/PoolAllocator.hpp>

StringMatch::StringMatch(Item& baseItem, TokenList&& tokens, std::string_view const& stringData)
	: Match(baseItem, std::move(tokens))
	, m_stringData(stringData)
{
	// NOP
}

std::string_view StringMatch::GetString() const
{
	return m_stringData;
}


typedef sprawl::memory::DynamicPoolAllocator<sizeof(StringMatch)> matchAllocator;

StringMatch* StringMatch::Create(Item& baseItem, TokenList&& tokens, const std::string_view& stringData)
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
