#pragma once

#include "Item.hpp"
#include <functional>
#include <string_view>
#include "../../../libocean/src/LibOcean.hpp"

class StringMatch;

class SpanItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter const&, StringMatch const&, OceanValue&)> Translator;

	SpanItem(std::string_view const& name, std::string_view const& startLiteral, std::string_view const& endLiteral, Translator const& translator);
	SpanItem(SpanItem const& other);

	int Find(std::string_view const& text);
	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	SpanItem& RestrictNewlines() { m_restrictNewlines = true; return *this; }
	SpanItem& Escape(std::string_view const& escapeValue) { m_escape = escapeValue; return *this; }
private:
	int _find(std::string_view const& text, int& startPosition, int& endPosition);
	std::string_view m_start;
	std::string_view m_end;
	Translator m_translator;
	std::string_view m_escape;
	bool m_restrictNewlines;
};
