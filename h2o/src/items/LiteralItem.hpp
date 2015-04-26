#pragma once

#include "Item.hpp"
#include <re2/re2.h>
#include <functional>

class RegexMatch;

class LiteralItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter const&, RegexMatch const&, int64_t&)> Translator;
	LiteralItem(sprawl::String const& name, char const* const regex, Translator const& translator);
	LiteralItem(LiteralItem const& other);

	size_t GroupNameToIndex(sprawl::String const& name);

	int Find(sprawl::StringLiteral const& text);
	virtual ::Match* Match(TokenList const& tokens) override;

	void StartHint(char const* const startHint) { m_startHint = sprawl::StringLiteral(startHint, strlen(startHint)); }

	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;
	bool GetLiteralValue(BytecodeWriter const& writer, RegexMatch const& match, int64_t& outValue);

	static sprawl::collections::HashMap<LiteralItem, sprawl::KeyAccessor<LiteralItem, sprawl::String>> ms_literals;
private:
	RE2 m_regex;
	char const* const m_regexBase;
	Translator m_translator;
	sprawl::StringLiteral m_startHint;
};

inline LiteralItem& AddLiteral(sprawl::String const& name, sprawl::StringLiteral const& regex, LiteralItem::Translator const& translator = nullptr)
{
	auto it = LiteralItem::ms_literals.insert(LiteralItem(name, regex.GetPtr(), translator), name);
	Item::ms_allItems.insert(&it.Value(), name);
	return it.Value();
}
