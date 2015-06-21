#pragma once

#include <sprawl/collections/HashMap.hpp>

#include "items/LiteralItem.hpp"
#include "items/SpanItem.hpp"
#include "items/RegexSpanItem.hpp"
#include "items/GroupItem.hpp"
#include "items/ExpressionItem.hpp"
#include <sprawl/collections/List.hpp>

class H2OCompiler
{
public:
	H2OCompiler();

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

	void EnableSignificantWhitespace() { m_significantWhitespace = true; }
private:
	sprawl::collections::BasicHashMap<sprawl::String, LiteralItem> m_literals;

	sprawl::collections::BasicHashMap<sprawl::String, SpanItem> m_spans;

	sprawl::collections::BasicHashMap<sprawl::String, RegexSpanItem> m_regexSpans;

	sprawl::collections::BasicHashMap<sprawl::String, GroupItem> m_groups;

	sprawl::collections::BasicHashMap<sprawl::String, ExpressionItem> m_expressions;
	sprawl::collections::HashSet<sprawl::String> m_expressionLiterals;

	sprawl::collections::BasicHashMap<sprawl::String, ExpressionItem::Translator> m_expressionTranslators;
	sprawl::collections::BasicHashMap<sprawl::String, LiteralItem::Translator> m_literalTranslators;
	sprawl::collections::BasicHashMap<sprawl::String, SpanItem::Translator> m_spanTranslators;

	sprawl::collections::BasicHashMap<sprawl::String, Item*> m_allItems;
	sprawl::collections::List<Item*> m_topLevelItems;

	bool m_significantWhitespace;
};
