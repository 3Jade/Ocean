#include "ExpressionMatch.hpp"


typedef sprawl::memory::DynamicPoolAllocator<sizeof(ExpressionMatch)> matchAllocator;

ExpressionMatch::ExpressionMatch(Item& item, size_t endIndex, ExpressionMatch::MatchMap&& matches)
	: Match(item, endIndex)
	, m_matches(std::move(matches))
{
	// NOP
}

ExpressionMatch::~ExpressionMatch()
{
	for(auto& vect : m_matches)
	{
		for(auto& subvect : vect)
		{
			for(auto& item : subvect)
			{
				item->Release();
			}
		}
	}
}

ExpressionMatch* ExpressionMatch::Create(Item& item, size_t endIndex, ExpressionMatch::MatchMap&& matches)
{
	ExpressionMatch* ret = (ExpressionMatch*)matchAllocator::alloc();
	new(ret) ExpressionMatch(item, endIndex, std::move(matches));
	return ret;
}

void ExpressionMatch::Release()
{
	this->~ExpressionMatch();
	matchAllocator::free(this);
}
