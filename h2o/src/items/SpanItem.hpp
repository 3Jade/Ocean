#pragma once

#include "Item.hpp"
#include <functional>
#include <sprawl/string/String.hpp>
#include "../../../libocean/src/LibOcean.hpp"

class StringMatch;

class SpanItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter const&, StringMatch const&, OceanValue&)> Translator;

	SpanItem(sprawl::String const& name, sprawl::StringLiteral const& startLiteral, sprawl::StringLiteral const& endLiteral, Translator const& translator);
	SpanItem(SpanItem const& other);

	int Find(sprawl::StringLiteral const& text);
	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	SpanItem& RestrictNewlines() { m_restrictNewlines = true; return *this; }
	SpanItem& Escape(sprawl::StringLiteral const& escapeValue) { m_escape = escapeValue; return *this; }
private:
	int _find(sprawl::StringLiteral const& text, int& startPosition, int& endPosition);
	sprawl::StringRef m_start;
	sprawl::StringRef m_end;
	Translator m_translator;
	sprawl::StringRef m_escape;
	bool m_restrictNewlines;
};
