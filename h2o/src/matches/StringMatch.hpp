#pragma once

#include "Match.hpp"

class StringMatch : public Match
{
public:
	StringMatch(Item& baseItem, size_t endIndex, sprawl::StringRef const& stringData);

	sprawl::String GetString() const;

	virtual void Print() override
	{
		printf("\"%s\"", m_stringData.GetPtr());
	}

	static StringMatch* Create(Item& baseItem, size_t endIndex, sprawl::StringRef const& stringData);
	virtual void Release();
private:
	sprawl::StringRef m_stringData;
};
