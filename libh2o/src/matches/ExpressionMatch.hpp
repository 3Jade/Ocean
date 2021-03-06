#pragma once

#include "Match.hpp"
#include <SkipProbe/SkipProbe.hpp>
#include <vector>

class ExpressionMatch : public Match
{
public:
	typedef SkipProbe::HashMap<std::string_view, std::vector<std::vector<Match*>>> MatchMap;

	ExpressionMatch(Item& item, TokenList&& tokens, MatchMap&& matches);
	~ExpressionMatch();

	MatchMap::const_iterator begin() const { return m_matches.cbegin(); }
	MatchMap::const_iterator end() const { return m_matches.cend(); }

	std::vector<std::vector<Match*>> const& operator[](std::string_view const& str) const;

	virtual MatchType GetType() const override { return MatchType::Expression; }

	virtual void Print() const override
	{
		bool first1 = true;
		for(auto kvp : m_matches)
		{
			if(!first1)
			{
				printf(",");
			}
			printf(" { %.*s : [", (int)kvp.key.length(), kvp.key.data());
			bool first = true;
			for(auto& vect : kvp.value)
			{
				if(!first)
				{
					printf(",");
				}
				printf(" [");
				bool first2 = true;
				for(auto& item : vect)
				{
					if(!first2)
					{
						printf(",");
					}
					printf(" ");
					item->Print();
					first2 = false;
				}
				first = false;
				printf(" ]");
			}
			printf(" ] }");
			first1 = false;
		}
	}

	static ExpressionMatch* Create(Item& item, TokenList&& tokens, MatchMap&& matches);
	virtual void Release() override;
private:
	MatchMap m_matches;
};

inline std::vector<std::vector<Match*>> const& ExpressionMatch::operator[](std::string_view const& str) const
{
	return m_matches.Get(str);
}
