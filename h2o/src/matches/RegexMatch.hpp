#pragma once

#include "Match.hpp"

namespace re2
{
	class StringPiece;
}
class LiteralItem;

class RegexMatch : public Match
{
public:
	RegexMatch(LiteralItem& baseItem, size_t endIndex, re2::StringPiece* stringPieces);
	~RegexMatch();

	sprawl::String Group(size_t index) const;
	sprawl::String Group(sprawl::String const& groupName) const;

	virtual void Print() override
	{
		printf("%.*s", (int)Group(0).length(), Group(0).c_str());
	}

	bool GetLiteralValue(BytecodeWriter& writer, int64_t& outValue) const;

	virtual void Release() override;
	static RegexMatch* Create(LiteralItem& baseItem, size_t endIndex, re2::StringPiece* stringPieces);
private:
	re2::StringPiece* m_stringPieces;
};
