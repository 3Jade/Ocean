#include "ExpressionMatch.hpp"


typedef sprawl::memory::DynamicPoolAllocator<sizeof(ExpressionMatch)> matchAllocator;

ExpressionMatch::ExpressionMatch(Item& item, TokenList&& tokens, ExpressionMatch::MatchMap&& matches)
	: Match(item, std::move(tokens))
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

ExpressionMatch* ExpressionMatch::Create(Item& item, TokenList&& tokens, ExpressionMatch::MatchMap&& matches)
{
	ExpressionMatch* ret = (ExpressionMatch*)matchAllocator::alloc();
	new(ret) ExpressionMatch(item, std::move(tokens), std::move(matches));
	return ret;
}

void ExpressionMatch::Release()
{
	this->~ExpressionMatch();
	matchAllocator::free(this);
}
