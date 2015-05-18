#pragma once

#include "Match.hpp"
#include <sprawl/collections/HashMap.hpp>
#include <vector>

class ExpressionMatch : public Match
{
public:
	typedef sprawl::collections::HashMap<std::vector<std::vector<Match*>>, sprawl::KeyAccessor<std::vector<std::vector<Match*>>, sprawl::String>> MatchMap;

	ExpressionMatch(Item& item, TokenList&& tokens, MatchMap&& matches);
	~ExpressionMatch();

	MatchMap::const_iterator begin() const { return m_matches.cbegin(); }
	MatchMap::const_iterator end() const { return m_matches.cend(); }

	std::vector<std::vector<Match*>> const& operator[](sprawl::String const& str) const;

	virtual MatchType GetType() const override { return MatchType::Expression; }

	virtual void Print() const override
	{
		bool first1 = true;
		for(auto it = m_matches.begin(); it.Valid(); ++it)
		{
			if(!first1)
			{
				printf(",");
			}
			printf(" { %.*s : [", (int)it.Key().length(), it.Key().c_str());
			bool first = true;
			for(auto& vect : it.Value())
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

inline std::vector<std::vector<Match*>> const& ExpressionMatch::operator[](sprawl::String const& str) const
{
	return m_matches.get(str);
}
