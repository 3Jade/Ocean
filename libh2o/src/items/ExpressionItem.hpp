#pragma once

#include "Item.hpp"
#include "../StringList.hpp"
#include <sprawl/collections/List.hpp>
#include <functional>

class ExpressionMatch;

class ExpressionItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter& writer, ExpressionMatch const&)> Translator;

	ExpressionItem(sprawl::String const& name, StringList&& values, Translator const& translator);

	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

private:
	StringList m_values;
	sprawl::collections::List<size_t> m_literals;
	Translator m_translator;
};
