#pragma once

#include "Item.hpp"
#include "../StringList.hpp"
#include <sprawl/collections/ForwardList.hpp>
#include <functional>

class ExpressionMatch;

class ExpressionItem : public Item
{
public:
	typedef std::function<bool(BytecodeWriter& writer, ExpressionMatch const&)> Translator;

	ExpressionItem(sprawl::String const& name, StringList&& values, Translator const& translator);

	virtual ::Match* Match(TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	static sprawl::collections::HashMap<ExpressionItem, sprawl::KeyAccessor<ExpressionItem, sprawl::String>> ms_expressions;
	static sprawl::collections::HashSet<sprawl::String> ms_expressionLiterals;
private:
	StringList m_values;
	sprawl::collections::ForwardList<size_t> m_literals;
	Translator m_translator;
};

inline ExpressionItem& AddExpression(sprawl::String const& name, std::initializer_list<const char* const> values, ExpressionItem::Translator const& translator = nullptr)
{
	StringList valueList;

	for(auto value : values)
	{
		valueList.PushBack(sprawl::StringLiteral(value, strlen(value)));
	}

	auto it = ExpressionItem::ms_expressions.insert(ExpressionItem(name, std::move(valueList), translator), name);
	Item::ms_allItems.insert(&it.Value(), name);
	return it.Value();
}
