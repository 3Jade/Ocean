#pragma once

#include "Item.hpp"
#include <re2/re2.h>
#include <functional>
#include "../../../libocean/src/LibOcean.hpp"

class RegexMatch;

class LiteralItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter const&, RegexMatch const&, OceanValue&)> Translator;
	LiteralItem(std::string_view const& name, std::string_view regex, Translator const& translator);
	LiteralItem(LiteralItem const& other);

	size_t GroupNameToIndex(std::string_view const& name);

	int Find(std::string_view const& text);
	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;

	void StartHint(char const* const startHint) { m_startHint = std::string_view(startHint, strlen(startHint)); }

	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;
	bool GetLiteralValue(BytecodeWriter const& writer, RegexMatch const& match, OceanValue& outValue);
private:
	RE2 m_regex;
	std::string_view m_regexBase;
	Translator m_translator;
	std::string_view m_startHint;
};
