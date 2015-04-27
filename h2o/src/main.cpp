#include "items/LiteralItem.hpp"
#include "items/SpanItem.hpp"
#include "items/RegexSpanItem.hpp"
#include "matches/Match.hpp"
#include "h2o.hpp"
#include "Logging.hpp"
#include "items/ExpressionItem.hpp"
#include "items/GroupItem.hpp"
#include "../../libocean/src/BytecodeWriter.hpp"
#include "../../libocean/src/LibOcean.hpp"

/**
 * @TODO: Token ID assignment, matching during Parse() by assigned ID, perhaps embed match into token?
 * @TODO: Re-entrant/multi-match groups to find most exact match
 * @TODO: Significant whitespace option
 */

bool Tokenize(sprawl::StringRef const& text, TokenList& tokens)
{
	sprawl::StringRef partialMatch("", 0);

	size_t lineStart = 0;
	size_t columnStart = 0;
	size_t offset = 0;
	size_t length = text.GetLength();
	size_t line = 0;
	size_t column = 0;

	bool matched = false;
	char const* const ptr = text.GetPtr();

	while(offset < length)
	{
		bool foundSpan = false;

		char c = *(ptr + offset);

		matched = false;

		if(isspace(c))
		{
			++column;
			if(c == '\n')
			{
				++line;
				column = 0;
			}
			++offset;
			continue;
		}

		for(auto& span : SpanItem::ms_spans)
		{
			int ret = span.Find(sprawl::StringRef(ptr + offset, text.GetLength() - offset));
			if(ret != -1)
			{
				sprawl::StringRef capturedToken(ptr + offset, ret);
				tokens.PushBack(Token(capturedToken, line, column));
				for(int i = 0; i < ret; ++i)
				{
					if(*(capturedToken.GetPtr() + i) == '\n')
					{
						++line;
						column = 0;
					}
					else
					{
						++column;
					}
				}
				offset += ret;
				lineStart = line;
				columnStart = column;
				foundSpan = true;
				matched = true;
				break;
			}
		}
		if(!foundSpan)
		{
			for(auto& span : RegexSpanItem::ms_regexSpans)
			{
				int ret = span.Find(sprawl::StringRef(ptr + offset, text.GetLength() - offset));
				if(ret != -1)
				{
					sprawl::StringRef capturedToken(ptr + offset, ret);
					tokens.PushBack(Token(capturedToken, line, column));
					for(int i = 0; i < ret; ++i)
					{
						if(*(capturedToken.GetPtr() + i) == '\n')
						{
							++line;
							column = 0;
						}
						else
						{
							++column;
						}
					}
					offset += ret;
					lineStart = line;
					columnStart = column;
					foundSpan = true;
					matched = true;
					break;
				}
			}
		}

		if(foundSpan)
		{
			continue;
		}

		partialMatch = sprawl::StringLiteral(ptr + offset, text.GetLength() - offset);
		for(auto& literal : ExpressionItem::ms_expressionLiterals)
		{
			if(literal.length() > partialMatch.GetLength())
			{
				continue;
			}
			if(!memcmp(partialMatch.GetPtr(), literal.c_str(), literal.length()))
			{
				char endChar = *(partialMatch.GetPtr() + literal.length() - 1);
				char nextChar = *(partialMatch.GetPtr() + literal.length());
				if(!isalnum(endChar) || !isalnum(nextChar))
				{
					matched = true;
					Token token(sprawl::StringLiteral(partialMatch.GetPtr(), literal.length()), lineStart, columnStart);
					tokens.PushBack(token);

					for(int i = 0; i < literal.length(); ++i)
					{
						if(literal[i] == '\n')
						{
							++line;
							column = 0;
							continue;
						}
						++column;
					}
					++column;

					offset += literal.length();
					lineStart = line;
					columnStart = column;
					break;
				}
			}
		}

		if(!matched)
		{
			for(auto& literal : LiteralItem::ms_literals)
			{
				int ret = literal.Find(partialMatch);
				if(ret != -1)
				{
					char endChar = *(partialMatch.GetPtr() + ret - 1);
					char nextChar = *(partialMatch.GetPtr() + ret);
					if(!isalnum(endChar) || !isalnum(nextChar))
					{
						matched = true;
						sprawl::StringLiteral literalMatch(ptr + offset, ret);
						Token token(literalMatch, lineStart, columnStart);
						tokens.PushBack(token);

						for(int i = 0; i < ret; ++i)
						{
							if(*(literalMatch.GetPtr() + i) == '\n')
							{
								++line;
								column = 0;
								continue;
							}
							++column;
						}
						++column;

						offset += ret;
						lineStart = line;
						columnStart = column;
						break;
					}
				}
			}
		}

		if(!matched)
		{
			int endOffset = offset+1;
			c = *(ptr + endOffset);
			while(isalnum(c))
			{
				++endOffset;
				c = *(ptr + endOffset);
			}
			partialMatch = sprawl::StringLiteral(ptr + offset, endOffset - offset);
			Token token(partialMatch, lineStart, columnStart);
			LogError(sprawl::String(sprawl::StringLiteral("Invalid syntax: `{}`")).format(sprawl::String(partialMatch)), TokenList(&token, 1));
			return false;
		}
	}

	return true;
}

#include <cstdio>

bool Compile(sprawl::StringLiteral const& text)
{
	TokenList tokens;
	if(!Tokenize(text, tokens))
	{
		return false;
	}
	int offset = 0;
	sprawl::collections::List<Match*> matches;
	while(offset < tokens.Length())
	{
		TokenList tokenList = tokens.Slice(offset, tokens.Length());
		bool found = false;
		for(auto& item : Item::ms_topLevelItems)
		{
			Match* match = item->Match(tokenList);
			if(match)
			{
				offset += match->End();
				matches.PushBack(match);
				found = true;
				break;
			}
		}
		if(!found)
		{
			LogError(sprawl::String(sprawl::StringLiteral("Invalid syntax: `{}`")).format(sprawl::String(tokens[0].Text())), tokens.Slice(offset, 1));
			return false;
		}
	}
	BytecodeWriter writer;
	for(auto& match : matches)
	{
		if(!match->Translate(writer))
		{
			return false;
		}
		match->Release();
	}
	//puts(out.c_str());
	FILE* f = fopen("out.occ", "w");
	sprawl::String result = writer.Finish();
	fwrite(result.c_str(), sizeof(char), result.length(), f);
	fclose(f);
	return true;
}

#include "matches/ExpressionMatch.hpp"
#include "matches/RegexMatch.hpp"
#include "matches/StringMatch.hpp"

bool declare_var(BytecodeWriter& writer, ExpressionMatch const& match)
{
	auto& initializers = match["initializer"][0];
	if(!initializers.empty())
	{
		if(!initializers[0]->Translate(writer))
		{
			return false;
		}
	}
	else
	{
		writer.Stack_Push(0L);
	}
	RegexMatch const* identifierMatch = static_cast<RegexMatch const*>(match["identifier"][0][0]);
	int64_t value;
	if(!identifierMatch->GetLiteralValue(writer, value))
	{
		return false;
	}
	writer.Variable_Declare(value);
	return true;
}

bool mult(BytecodeWriter& writer, ExpressionMatch const& match)
{
	if(!match["LHS"][0][0]->Translate(writer))
	{
		return false;
	}
	if(!match["RHS"][0][0]->Translate(writer))
	{
		return false;
	}
	writer.Function_Call("mult");
	return true;
}

bool add(BytecodeWriter& writer, ExpressionMatch const& match)
{
	if(!match["LHS"][0][0]->Translate(writer))
	{
		return false;
	}
	if(!match["RHS"][0][0]->Translate(writer))
	{
		return false;
	}
	writer.Function_Call("add");
	return true;
}

bool ReadDecimal(BytecodeWriter const& /*writer*/, RegexMatch const& match, int64_t& outValue)
{
	outValue = atoi(match.Group(0).c_str());
	return true;
}

bool ReadString(BytecodeWriter const& writer, StringMatch const& match, int64_t& outValue)
{
	outValue = writer.GetStringOffset(match.GetString());
	return true;
}

#include <sprawl/time/time.hpp>

int main()
{
	Ocean::Install();

	AddSpan("dqstring", "\"", "\"", ReadString).Escape(R"(\")").RestrictNewlines();
	AddSpan("sqstring", "'", "'", ReadString).Escape(R"(\')").RestrictNewlines();
	AddRegexSpan("rawstring", "R(\\[.*\\])?\"", R"("\1)", ReadString).StartHint("R");
	AddGroup("string", { "dqstring", "sqstring", "rawstring" });

	AddLiteral("identifier", R"([a-zA-Z][a-zA-Z0-9]*)");

	AddLiteral("decfloat", R"([0-9]+\.[0-9]+)", ReadDecimal);
//	AddLiteral("hexfloat", R"(0x[0-9a-fA-F]+\.[0-9a-fA-F]+)", ReadNumber).StartHint("0x");
//	AddLiteral("binfloat", R"(0b[01]+\.[01]+)", ReadNumber).StartHint("0b");
//	AddLiteral("octfloat", R"(0o[0-7]+\.[0-7]+)", ReadNumber).StartHint("0o");
	AddGroup("float", { "decfloat" });//, "hexfloat", "binfloat", "octfloat"});

	AddLiteral("decinteger", R"([0-9]+)", ReadDecimal);
//	AddLiteral("hexinteger", R"(0x[0-9a-fA-F]+)", ReadNumber).StartHint("0x");
//	AddLiteral("bininteger", R"(0b[01]+)", ReadNumber).StartHint("0b");
//	AddLiteral("octinteger", R"(0o[0-7]+)", ReadNumber).StartHint("0o");
	AddGroup("integer", { "decinteger" });//, "hexinteger", "bininteger", "octinteger"});

	AddGroup("number", {"integer", "float"});

	AddGroup("value", {"identifier", "number", "string"});

	AddExpression("grouping", { "(", "$expression", ")" });

	AddExpression("mult", {"$expression<LHS>", "*", "$expression<RHS>"}, mult);
	AddExpression("add", {"$expression<LHS>", "+", "$expression<RHS>"}, add);

	AddGroup("binary_operator", { "add", "mult" });
	AddGroup("expression", {"binary_operator", "grouping", "value"});

	AddExpression("initializer", {"=", "$expression"});
	AddExpression("variable_declaration", { "var", "$identifier", "$?initializer" }, declare_var);

	AddTopLevelItem("variable_declaration");

	char const* const text = "var i = ( 5 + 10 ) * (10+15*6)";

	g_currentSource = text;
	g_currentFile = "<string>";

//	int64_t now = sprawl::time::SteadyNow(sprawl::time::Resolution::Milliseconds);
//	for(int i = 0; i < 10000; ++i)
//	{
//		Compile(sprawl::StringLiteral(text, strlen(text)));
//	}
//	int64_t elapsed = sprawl::time::SteadyNow(sprawl::time::Resolution::Milliseconds) - now;
//	printf("Elapsed: %ld\n", elapsed);

	Compile(sprawl::StringLiteral(text, strlen(text)));

	return 0;
}
