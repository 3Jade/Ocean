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

	RegexSpanItem(std::string_view const& name, std::string_view const& startRegex, std::string_view const& endLiteral, Translator const& translator);
	RegexSpanItem(RegexSpanItem const& other);

	int Find(std::string_view const& text);
	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	RegexSpanItem& RestrictNewlines() { m_restrictNewlines = true; return *this; }
	RegexSpanItem& Escape(std::string_view const& escapeValue) { m_escape = escapeValue; return *this; }
	RegexSpanItem& StartHint(char const* const literal) { m_startHint = std::string_view(literal, strlen(literal)); return *this; }
private:
	int _find(std::string_view const& text, int& startPosition, int& endPosition);
	RE2 m_startRegex;
	std::string_view m_startRegexBase;
	std::string_view m_end;
	Translator m_translator;
	std::string_view  m_escape;
	bool m_restrictNewlines;
	std::string_view m_startHint;
};
