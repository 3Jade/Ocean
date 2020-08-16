#pragma once

#include <SkipProbe/SkipProbe.hpp>
#include <list>
#include <unordered_set>

#include "items/LiteralItem.hpp"
#include "items/SpanItem.hpp"
#include "items/RegexSpanItem.hpp"
#include "items/GroupItem.hpp"
#include "items/ExpressionItem.hpp"

class H2OCompiler
{
public:
	H2OCompiler();

	~H2OCompiler()
	{
		for (auto& literal : m_literals)
		{
			delete literal.value;
		}
		for (auto& span : m_spans)
		{
			delete span.value;
		}
		for (auto& regexSpan : m_regexSpans)
		{
			delete regexSpan.value;
		}
		for (auto& group : m_groups)
		{
			delete group.value;
		}
		for (auto& expression : m_expressions)
		{
			delete expression.value;
		}
	}

	bool Tokenize(std::string_view const& text, TokenList& tokens);
	bool Parse(TokenList const& tokens, std::string& output);

	LiteralItem& AddLiteral(std::string_view const& name, std::string_view const& regex, LiteralItem::Translator const& translator = nullptr);
	SpanItem& AddSpan(std::string_view const& name, std::string_view const& startLiteral, std::string_view const& endLiteral, SpanItem::Translator const& translator);
	RegexSpanItem& AddRegexSpan(std::string_view const& name, std::string_view const& startRegex, std::string_view const& endLiteral, RegexSpanItem::Translator const& translator);

	ExpressionItem& AddExpression(std::string_view const& name, std::initializer_list<const char* const> values, ExpressionItem::Translator const& translator = nullptr);
	ExpressionItem& AddExpression(const std::string_view& name, StringList&& valueList, const ExpressionItem::Translator& translator = nullptr);

	GroupItem& AddGroup(std::string_view const& name, std::initializer_list<const char* const> names);
	GroupItem& AddGroup(const std::string_view& name, std::list<std::string_view>&& nameList);

	void AddTopLevelItem(std::string_view const& itemName);

	Item* GetItem(std::string_view const& itemName) const { return m_allItems.Get(itemName); }

	void AddLiteralTranslator(std::string_view const& name, LiteralItem::Translator const& translator);
	void AddExpressionTranslator(std::string_view const& name, ExpressionItem::Translator const& translator);
	void AddSpanTranslator(std::string_view const& name, SpanItem::Translator const& translator);

	LiteralItem::Translator GetLiteralTranslator(std::string_view const& name);
	ExpressionItem::Translator GetExpressionTranslator(std::string_view const& name);
	SpanItem::Translator GetSpanTranslator(std::string_view const& name);

	void EnableSignificantWhitespace() { m_significantWhitespace = true; }
private:
	SkipProbe::HashMap<std::string_view, LiteralItem*> m_literals;

	SkipProbe::HashMap<std::string_view, SpanItem*> m_spans;

	SkipProbe::HashMap<std::string_view, RegexSpanItem*> m_regexSpans;

	SkipProbe::HashMap<std::string_view, GroupItem*> m_groups;

	SkipProbe::HashMap<std::string_view, ExpressionItem*> m_expressions;
	std::unordered_set<std::string_view> m_expressionLiterals;

	SkipProbe::HashMap<std::string_view, ExpressionItem::Translator> m_expressionTranslators;
	SkipProbe::HashMap<std::string_view, LiteralItem::Translator> m_literalTranslators;
	SkipProbe::HashMap<std::string_view, SpanItem::Translator> m_spanTranslators;

	SkipProbe::HashMap<std::string_view, Item*> m_allItems;
	std::list<Item*> m_topLevelItems;

	bool m_significantWhitespace;
};
