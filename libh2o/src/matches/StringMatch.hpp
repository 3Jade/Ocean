#pragma once

#include "Match.hpp"

class StringMatch : public Match
{
public:
	StringMatch(Item& baseItem, TokenList&& tokens, std::string_view const& stringData);

	std::string_view GetString() const;

	virtual MatchType GetType() const override { return MatchType::String; }

	virtual void Print() const override
	{
		printf("\"%s\"", m_stringData.data());
	}

	static StringMatch* Create(Item& baseItem, TokenList&& tokens, std::string_view const& stringData);
	virtual void Release() override;
private:
	std::string_view m_stringData;
};
