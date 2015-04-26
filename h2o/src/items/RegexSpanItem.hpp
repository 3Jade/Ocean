#pragma once

#include "Item.hpp"
#include <re2/re2.h>
#include <functional>

class StringMatch;

class RegexSpanItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter const&, StringMatch const&, int64_t&)> Translator;

	RegexSpanItem(sprawl::String const& name, char const* const startRegex, char const* const endLiteral, Translator const& translator);
	RegexSpanItem(RegexSpanItem const& other);

	int Find(sprawl::StringLiteral const& text);
	virtual ::Match* Match(TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	RegexSpanItem& RestrictNewlines() { m_restrictNewlines = true; return *this; }
	RegexSpanItem& Escape(char const* const escapeValue) { m_escape = sprawl::StringLiteral(escapeValue, strlen(escapeValue)); return *this; }
	RegexSpanItem& StartHint(char const* const literal) { m_startHint = sprawl::StringLiteral(literal, strlen(literal)); return *this; }

	static sprawl::collections::HashMap<RegexSpanItem, sprawl::KeyAccessor<RegexSpanItem, sprawl::String>> ms_regexSpans;
private:
	int _find(sprawl::StringLiteral const& text, int& startPosition, int& endPosition);
	RE2 m_startRegex;
	char const* const m_startRegexBase;
	char const* const m_end;
	Translator m_translator;
	sprawl::StringLiteral  m_escape;
	bool m_restrictNewlines;
	sprawl::StringLiteral m_startHint;
};

inline RegexSpanItem& AddRegexSpan(sprawl::String const& name, char const* const startRegex, char const* const endLiteral, RegexSpanItem::Translator const& translator)
{
	auto it = RegexSpanItem::ms_regexSpans.insert(RegexSpanItem(name, startRegex, endLiteral, translator), name);
	Item::ms_allItems.insert(&it.Value(), name);
	return it.Value();
}
