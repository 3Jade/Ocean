#pragma once

#include "items/LiteralItem.hpp"
#include "items/SpanItem.hpp"
#include "items/RegexSpanItem.hpp"
#include "items/GroupItem.hpp"
#include "items/ExpressionItem.hpp"

class H2OCompiler
{
public:
	bool Tokenize(sprawl::StringLiteral const& text, TokenList& tokens);
	bool Parse(TokenList const& tokens, sprawl::String& output);

	LiteralItem& AddLiteral(sprawl::String const& name, sprawl::StringLiteral const& regex, LiteralItem::Translator const& translator = nullptr);
	SpanItem& AddSpan(sprawl::String const& name, sprawl::StringLiteral const& startLiteral, sprawl::StringLiteral const& endLiteral, SpanItem::Translator const& translator);
	RegexSpanItem& AddRegexSpan(sprawl::String const& name, sprawl::StringLiteral const& startRegex, sprawl::StringLiteral const& endLiteral, RegexSpanItem::Translator const& translator);

	ExpressionItem& AddExpression(sprawl::String const& name, std::initializer_list<const char* const> values, ExpressionItem::Translator const& translator = nullptr);
	ExpressionItem& AddExpression(const sprawl::String& name, StringList&& valueList, const ExpressionItem::Translator& translator = nullptr);

	GroupItem& AddGroup(sprawl::String const& name, std::initializer_list<const char* const> names);
	GroupItem& AddGroup(const sprawl::String& name, sprawl::collections::List<sprawl::StringLiteral>&& nameList);

	void AddTopLevelItem(sprawl::String const& itemName);

	Item* GetItem(sprawl::String const& itemName) const { return m_allItems.get(itemName); }

	void AddLiteralTranslator(sprawl::String const& name, LiteralItem::Translator const& translator);
	void AddExpressionTranslator(sprawl::String const& name, ExpressionItem::Translator const& translator);
	void AddSpanTranslator(sprawl::String const& name, SpanItem::Translator const& translator);

	LiteralItem::Translator GetLiteralTranslator(sprawl::String const& name);
	ExpressionItem::Translator GetExpressionTranslator(sprawl::String const& name);
	SpanItem::Translator GetSpanTranslator(sprawl::String const& name);
private:
	sprawl::collections::HashMap<LiteralItem, sprawl::KeyAccessor<LiteralItem, sprawl::String>> m_literals;

	sprawl::collections::HashMap<SpanItem, sprawl::KeyAccessor<SpanItem, sprawl::String>> m_spans;

	sprawl::collections::HashMap<RegexSpanItem, sprawl::KeyAccessor<RegexSpanItem, sprawl::String>> m_regexSpans;

	sprawl::collections::HashMap<GroupItem, sprawl::KeyAccessor<GroupItem, sprawl::String>> m_groups;

	sprawl::collections::HashMap<ExpressionItem, sprawl::KeyAccessor<ExpressionItem, sprawl::String>> m_expressions;
	sprawl::collections::HashSet<sprawl::String> m_expressionLiterals;

	sprawl::collections::HashMap<ExpressionItem::Translator, sprawl::KeyAccessor<ExpressionItem::Translator, sprawl::String>> m_expressionTranslators;
	sprawl::collections::HashMap<LiteralItem::Translator, sprawl::KeyAccessor<LiteralItem::Translator, sprawl::String>> m_literalTranslators;
	sprawl::collections::HashMap<SpanItem::Translator, sprawl::KeyAccessor<SpanItem::Translator, sprawl::String>> m_spanTranslators;

	sprawl::collections::HashMap<Item*, sprawl::KeyAccessor<Item*, sprawl::String>> m_allItems;
	sprawl::collections::ForwardList<Item*> m_topLevelItems;
};
