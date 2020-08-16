#include "H2OMetaLanguage.hpp"
#include "H2OCompiler.hpp"
#include "matches/ExpressionMatch.hpp"
#include "matches/RegexMatch.hpp"
#include "matches/StringMatch.hpp"
#include "StringLiteral.hpp"

namespace H2OMetaLanguageStatic
{
	static thread_local H2OCompiler* currentCompiler = nullptr;

	static bool AddLiteral(BytecodeWriter& /*writer*/, ExpressionMatch const& match)
	{
		ExpressionMatch& identifierFunctionMatch = static_cast<ExpressionMatch&>(*match["identifiercommafunction"][0][0]);

		LiteralItem::Translator function;
		auto& commaFunction = identifierFunctionMatch["commafunction"][0];
		if(!commaFunction.empty())
		{
			ExpressionMatch& commaExpression = static_cast<ExpressionMatch&>(*commaFunction[0]);
			ExpressionMatch& functionMatch = static_cast<ExpressionMatch&>(*commaExpression["setfunction"][0][0]);
			function = currentCompiler->GetLiteralTranslator(static_cast<RegexMatch*>(functionMatch["identifier"][0][0])->Group(0));
		}

		RegexMatch* identifier = static_cast<RegexMatch*>(identifierFunctionMatch["identifier"][0][0]);
		StringMatch* string = static_cast<StringMatch*>(match["string"][0][0]);

		currentCompiler->AddLiteral(identifier->Group(0), string->GetString(), function);
		return true;
	}

	static bool AddGroup(BytecodeWriter& /*writer*/, ExpressionMatch const& match)
	{
		RegexMatch* identifier = static_cast<RegexMatch*>(match["identifier"][0][0]);
		ExpressionMatch& groupliteral = static_cast<ExpressionMatch&>(*match["groupliteral"][0][0]);

		std::list<std::string_view> group;
		for(auto& groupMember : groupliteral["identifier"][0])
		{
			RegexMatch& groupMatch = static_cast<RegexMatch&>(*groupMember);
			group.push_back(groupMatch.Group(0));
		}

		currentCompiler->AddGroup(identifier->Group(0), std::move(group));
		return true;
	}

	static bool AddExpression(BytecodeWriter& /*writer*/, ExpressionMatch const& match)
	{
		ExpressionMatch& identifierFunctionMatch = static_cast<ExpressionMatch&>(*match["identifiercommafunction"][0][0]);

		ExpressionItem::Translator function;
		auto& commaFunction = identifierFunctionMatch["commafunction"][0];
		if(!commaFunction.empty())
		{
			ExpressionMatch& commaExpression = static_cast<ExpressionMatch&>(*commaFunction[0]);
			ExpressionMatch& functionMatch = static_cast<ExpressionMatch&>(*commaExpression["setfunction"][0][0]);
			function = currentCompiler->GetExpressionTranslator(static_cast<RegexMatch*>(functionMatch["identifier"][0][0])->Group(0));
		}

		RegexMatch* identifier = static_cast<RegexMatch*>(identifierFunctionMatch["identifier"][0][0]);

		ExpressionMatch& expressionLiteral = static_cast<ExpressionMatch&>(*match["expressionliteral"][0][0]);

		StringList list;
		for(auto& expressionComponent : expressionLiteral["expressioncomponent"][0])
		{
			if(expressionComponent->GetType() == MatchType::Regex)
			{
				RegexMatch& expressionMatch = static_cast<RegexMatch&>(*expressionComponent);
				Item& item = expressionMatch.GetBaseItem();
				if(item.GetName() == StringLiteral("indent"))
				{
					list.PushBack(std::string_view("\t", 1));
				}
				else if(item.GetName() == StringLiteral("dedent"))
				{
					list.PushBack(std::string_view("\r", 1));
				}
				else
				{
					list.PushBack(expressionMatch.Group(0));
				}
			}
			else
			{
				StringMatch& expressionMatch = static_cast<StringMatch&>(*expressionComponent);

				list.PushBack(expressionMatch.GetString());
			}
		}

		currentCompiler->AddExpression(identifier->Group(0), std::move(list), function);
		return true;
	}

	static bool AddSpan(BytecodeWriter& /*writer*/, ExpressionMatch const& match)
	{
		ExpressionMatch& identifierSpanArgMatch = static_cast<ExpressionMatch&>(*match["identifiercommaspanarg"][0][0]);

		RegexMatch* identifier = static_cast<RegexMatch*>(identifierSpanArgMatch["identifier"][0][0]);

		SpanItem::Translator function;

		std::string_view start(nullptr, 0);
		std::string_view end(nullptr, 0);
		std::string_view escape(nullptr, 0);
		std::string_view noNewlines(nullptr, 0);
		auto& commaSpanArg = identifierSpanArgMatch["commaspanarg"][0];
		for(int i = 0; i < commaSpanArg.size(); ++i)
		{
			ExpressionMatch& commaExpression = static_cast<ExpressionMatch&>(*commaSpanArg[i]);
			ExpressionMatch& spanarg = static_cast<ExpressionMatch&>(*commaExpression["spanargs"][0][0]);

			std::string firstToken(spanarg.Tokens()[0].Text());

			if(firstToken == StringLiteral("function"))
			{
				function = currentCompiler->GetSpanTranslator(static_cast<RegexMatch*>(spanarg["identifier"][0][0])->Group(0));
			}
			else if(firstToken == StringLiteral("start"))
			{
				start = static_cast<StringMatch*>(spanarg["string"][0][0])->GetString();
			}
			else if(firstToken == StringLiteral("end"))
			{
				end = static_cast<StringMatch*>(spanarg["string"][0][0])->GetString();
			}
			else if(firstToken == StringLiteral("escape"))
			{
				escape = static_cast<StringMatch*>(spanarg["string"][0][0])->GetString();
			}
			else
			{
				noNewlines = static_cast<RegexMatch*>(spanarg["bool"][0][0])->Group(0);
			}
		}

		if(start.length() == 0 || end.length() == 0)
		{
			return false;
		}

		auto item = currentCompiler->AddSpan(identifier->Group(0), start, end, function);

		if(escape.length() != 0)
		{
			item.Escape(escape);
		}

		if(noNewlines.length() != 0 && noNewlines.data()[0] == 't')
		{
			item.RestrictNewlines();
		}
		return true;
	}

	static bool AddRegexSpan(BytecodeWriter& /*writer*/, ExpressionMatch const& match)
	{
		ExpressionMatch& identifierSpanArgMatch = static_cast<ExpressionMatch&>(*match["identifiercommaspanarg"][0][0]);

		RegexMatch* identifier = static_cast<RegexMatch*>(identifierSpanArgMatch["identifier"][0][0]);

		RegexSpanItem::Translator function;

		std::string_view start(nullptr, 0);
		std::string_view end(nullptr, 0);
		std::string_view escape(nullptr, 0);
		std::string_view noNewlines(nullptr, 0);
		auto& commaSpanArg = identifierSpanArgMatch["commaspanarg"][0];
		for(int i = 0; i < commaSpanArg.size(); ++i)
		{
			ExpressionMatch& commaExpression = static_cast<ExpressionMatch&>(*commaSpanArg[i]);
			ExpressionMatch& spanarg = static_cast<ExpressionMatch&>(*commaExpression["spanargs"][0][0]);

			std::string_view const& firstToken = spanarg.Tokens()[0].Text();

			if(firstToken == StringLiteral("function"))
			{
				function = currentCompiler->GetSpanTranslator(static_cast<RegexMatch*>(spanarg["identifier"][0][0])->Group(0));
			}
			else if(firstToken == StringLiteral("start"))
			{
				start = static_cast<StringMatch*>(spanarg["string"][0][0])->GetString();
			}
			else if(firstToken == StringLiteral("end"))
			{
				end = static_cast<StringMatch*>(spanarg["string"][0][0])->GetString();
			}
			else if(firstToken == StringLiteral("escape"))
			{
				escape = static_cast<StringMatch*>(spanarg["string"][0][0])->GetString();
			}
			else
			{
				noNewlines = static_cast<RegexMatch*>(spanarg["bool"][0][0])->Group(0);
			}
		}

		if(start.length() == 0 || end.length() == 0)
		{
			return false;
		}

		auto item = currentCompiler->AddRegexSpan(identifier->Group(0), start, end, function);

		if(escape.length() != 0)
		{
			item.Escape(escape);
		}

		if(noNewlines.length() != 0 && noNewlines.data()[0] == 't')
		{
			item.RestrictNewlines();
		}
		return true;
	}

	static bool SetTopLevelItems(BytecodeWriter& /*writer*/, ExpressionMatch const& match)
	{
		ExpressionMatch& groupliteral = static_cast<ExpressionMatch&>(*match["groupliteral"][0][0]);

		for(auto& groupMember : groupliteral["identifier"][0])
		{
			RegexMatch& groupMatch = static_cast<RegexMatch&>(*groupMember);
			currentCompiler->AddTopLevelItem(groupMatch.Group(0));
		}

		return true;
	}

	static bool EnableSignificantWhitespace(BytecodeWriter& /*writer*/, ExpressionMatch const& /*match*/)
	{
		currentCompiler->EnableSignificantWhitespace();
		return true;
	}
}

bool ProcessH2OFile(char const* data, long dataSize, H2OCompiler& compilerToCreate)
{
	using namespace H2OMetaLanguageStatic;

	currentCompiler = &compilerToCreate;

	H2OCompiler compiler;
	compiler.AddSpan("comment", "#", "", nullptr).RestrictNewlines();

	compiler.AddSpan("dqstring", "\"", "\"", nullptr).Escape(R"(\")").RestrictNewlines();
	compiler.AddSpan("sqstring", "'", "'", nullptr).Escape(R"(\')").RestrictNewlines();
	compiler.AddRegexSpan("rawdqstring", "R(\\[.*\\])?\"", R"("\1)", nullptr);
	compiler.AddRegexSpan("rawsqstring", "R(\\[.*\\])?'", R"('\1)", nullptr);
	compiler.AddGroup("string", { "dqstring", "sqstring", "rawsqstring", "rawdqstring" });

	compiler.AddLiteral("true", "true");
	compiler.AddLiteral("false", "false");
	compiler.AddGroup("bool", {"true", "false"});

	compiler.AddLiteral("indent", ">>");
	compiler.AddLiteral("dedent", "<<");

	compiler.AddLiteral("identifier", R"([a-zA-Z_][a-zA-Z0-9_]*)");
	compiler.AddLiteral("reference", R"(\$[+?*!]?[a-zA-Z_][a-zA-Z0-9_]*(\<[a-zA-Z_][a-zA-Z0-9_]*\>)?)");

	compiler.AddExpression("setfunction", { "function", "=", "$identifier"});
	compiler.AddExpression("setstart", { "start", "=", "$string"});
	compiler.AddExpression("setend", { "end", "=", "$string"});
	compiler.AddExpression("setescape", { "escape", "=", "$string"});
	compiler.AddExpression("setnonewlines", { "nonewlines", "=", "$bool"});

	compiler.AddGroup("spanargs", { "setfunction", "setstart", "setend", "setescape", "setnonewlines" });

	compiler.AddExpression("groupliteral", { "[", "$+identifier", "]"});

	compiler.AddGroup("expressioncomponent", { "string", "reference", "indent", "dedent" });
	compiler.AddExpression("expressionliteral", { "{", "$+expressioncomponent", "}" });

	compiler.AddExpression("commafunction", { ",", "$setfunction" });
	compiler.AddExpression("commaspanarg", { ",", "$spanargs" });

	compiler.AddExpression("identifiercommafunction", {"$identifier", "$?commafunction"});
	compiler.AddExpression("identifiercommaspanarg", {"$identifier", "$*commaspanarg"});

	compiler.AddExpression("AddLiteral", { "Literal", "(", "$identifiercommafunction", ",", "$string", ")", ";"}, AddLiteral);
	compiler.AddExpression("AddGroup", { "Group", "(", "$identifier", ",", "$groupliteral", ")", ";"}, AddGroup);
	compiler.AddExpression("AddExpression", { "Expression", "(", "$identifiercommafunction", ",", "$expressionliteral", ")", ";"}, AddExpression);
	compiler.AddExpression("AddSpan", { "Span", "(", "$identifiercommaspanarg", ")", ";"}, AddSpan);
	compiler.AddExpression("AddRegexSpan", { "RegexSpan", "(", "$identifiercommaspanarg", ")", ";"}, AddRegexSpan);
	compiler.AddExpression("SetTopLevelItems", { "TopLevelItems", "(", "$groupliteral", ")", ";"}, SetTopLevelItems);
	compiler.AddExpression("EnableSignificantWhitespace", { "EnableSignificantWhitespace", "(", ")", ";"}, EnableSignificantWhitespace);

	compiler.AddTopLevelItem("comment");
	compiler.AddTopLevelItem("AddLiteral");
	compiler.AddTopLevelItem("AddGroup");
	compiler.AddTopLevelItem("AddExpression");
	compiler.AddTopLevelItem("AddSpan");
	compiler.AddTopLevelItem("AddRegexSpan");
	compiler.AddTopLevelItem("SetTopLevelItems");
	compiler.AddTopLevelItem("EnableSignificantWhitespace");

	TokenList tokens;
	if(!compiler.Tokenize(std::string_view(data, dataSize), tokens))
	{
		currentCompiler = nullptr;
		return false;
	}

	std::string ignoredOutput;
	if(!compiler.Parse(tokens, ignoredOutput))
	{
		currentCompiler = nullptr;
		return false;
	}

	currentCompiler = nullptr;
	return true;
}
