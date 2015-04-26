#pragma once

#include "Item.hpp"
#include <functional>
#include <sprawl/string/String.hpp>

class StringMatch;

class SpanItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter const&, StringMatch const&, int64_t&)> Translator;

	SpanItem(sprawl::String const& name, char const* const startLiteral, char const* const endLiteral, Translator const& translator);
	SpanItem(SpanItem const& other);

	int Find(sprawl::StringLiteral const& text);
	virtual ::Match* Match(TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	SpanItem& RestrictNewlines() { m_restrictNewlines = true; return *this; }
	SpanItem& Escape(char const* const escapeValue) { m_escape = sprawl::StringLiteral(escapeValue, strlen(escapeValue)); return *this; }

	static sprawl::collections::HashMap<SpanItem, sprawl::KeyAccessor<SpanItem, sprawl::String>> ms_spans;
private:
	int _find(sprawl::StringLiteral const& text, int& startPosition, int& endPosition);
	sprawl::StringRef m_start;
	sprawl::StringRef m_end;
	Translator m_translator;
	sprawl::StringRef m_escape;
	bool m_restrictNewlines;
};

inline SpanItem& AddSpan(sprawl::String const& name, char const* const startLiteral, char const* const endLiteral, SpanItem::Translator const& translator)
{
	auto it = SpanItem::ms_spans.insert(SpanItem(name, startLiteral, endLiteral, translator), name);
	Item::ms_allItems.insert(&it.Value(), name);
	return it.Value();
}
