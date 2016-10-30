#pragma once

#include "Match.hpp"
#include "../../../libocean/src/LibOcean.hpp"

namespace re2
{
	class StringPiece;
}
class LiteralItem;

class RegexMatch : public Match
{
public:
	RegexMatch(LiteralItem& baseItem, TokenList&& tokens, re2::StringPiece* stringPieces);
	~RegexMatch();

	virtual MatchType GetType() const override { return MatchType::Regex; }

	sprawl::StringLiteral Group(size_t index) const;
	sprawl::StringLiteral Group(sprawl::String const& groupName) const;

	virtual void Print() const override
	{
		printf("%.*s", (int)Group(0).GetLength(), Group(0).GetPtr());
	}

	bool GetLiteralValue(BytecodeWriter& writer, OceanValue& outValue) const;

	virtual void Release() override;
	static RegexMatch* Create(LiteralItem& baseItem, TokenList&& tokens, re2::StringPiece* stringPieces);
private:
	re2::StringPiece* m_stringPieces;
};
