#pragma once

#include "Item.hpp"
#include <re2/re2.h>
#include <functional>
#include "../../../libocean/src/LibOcean.hpp"

class StringMatch;

class RegexSpanItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter const&, StringMatch const&, OceanValue&)> Translator;

	RegexSpanItem(sprawl::String const& name, sprawl::StringLiteral const& startRegex, sprawl::StringLiteral const& endLiteral, Translator const& translator);
	RegexSpanItem(RegexSpanItem const& other);

	int Find(sprawl::StringLiteral const& text);
	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	RegexSpanItem& RestrictNewlines() { m_restrictNewlines = true; return *this; }
	RegexSpanItem& Escape(sprawl::StringLiteral const& escapeValue) { m_escape = escapeValue; return *this; }
	RegexSpanItem& StartHint(char const* const literal) { m_startHint = sprawl::StringLiteral(literal, strlen(literal)); return *this; }
private:
	int _find(sprawl::StringLiteral const& text, int& startPosition, int& endPosition);
	RE2 m_startRegex;
	sprawl::StringLiteral m_startRegexBase;
	sprawl::StringLiteral m_end;
	Translator m_translator;
	sprawl::StringLiteral  m_escape;
	bool m_restrictNewlines;
	sprawl::StringLiteral m_startHint;
};
