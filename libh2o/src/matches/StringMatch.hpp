#pragma once

#include "Match.hpp"

class StringMatch : public Match
{
public:
	StringMatch(Item& baseItem, TokenList&& tokens, sprawl::StringRef const& stringData);

	sprawl::StringRef GetString() const;

	virtual MatchType GetType() const override { return MatchType::String; }

	virtual void Print() const override
	{
		printf("\"%s\"", m_stringData.GetPtr());
	}

	static StringMatch* Create(Item& baseItem, TokenList&& tokens, sprawl::StringRef const& stringData);
	virtual void Release();
private:
	sprawl::StringRef m_stringData;
};
