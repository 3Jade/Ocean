#include "StringMatch.hpp"
#include "../items/RegexSpanItem.hpp"

StringMatch::StringMatch(Item& baseItem, size_t endIndex, sprawl::StringRef const& stringData)
	: Match(baseItem, endIndex)
	, m_stringData(stringData)
{
	// NOP
}

sprawl::String StringMatch::GetString() const
{
	return m_stringData;
}


typedef sprawl::memory::DynamicPoolAllocator<sizeof(StringMatch)> matchAllocator;

StringMatch* StringMatch::Create(Item& baseItem, size_t endIndex, const sprawl::StringRef& stringData)
{
	StringMatch* ret = (StringMatch*)matchAllocator::alloc();
	new(ret) StringMatch(baseItem, endIndex, stringData);
	return ret;
}

void StringMatch::Release()
{
	this->~StringMatch();
	matchAllocator::free(this);
}
