#include "H2OCompiler.hpp"
#include "Logging.hpp"
#include "matches/Match.hpp"
#include "../../libocean/src/BytecodeWriter.hpp"
#include "../../libocean/src/LibOcean.hpp"
#include <vector>

H2OCompiler::H2OCompiler()
	: m_significantWhitespace(false)
{
	// NOP
}

bool H2OCompiler::Tokenize(sprawl::StringRef const& text, TokenList& tokens)
{
	sprawl::StringRef partialMatch("", 0);

	int lineStart = -1;
	int columnStart = -1;
	size_t offset = 0;
	size_t length = text.GetLength();
	size_t line = 0;
	size_t column = 0;

	bool matched = false;
	char const* const ptr = text.GetPtr();

	std::vector<sprawl::StringLiteral> indents;
	size_t indentStartOffset = 0;

	if(m_significantWhitespace)
	{
		char c = *(ptr + offset);
		while(isspace(c))
		{
			++column;

			if(c == '\n')
			{
				++line;
				column = 0;
				indentStartOffset = offset + 1;
			}
			else if(c == '\r')
			{
				indentStartOffset = offset + 1;
			}
			else if(c != '\t' && c != ' ')
			{
				Token tok(sprawl::StringLiteral(ptr + offset, 1), line, column);
				LogError("Invalid character", TokenList(&tok, 1));
				return false;
			}
			++offset;
			c = *(ptr + offset);
		}
		indents.push_back(sprawl::StringLiteral(ptr + indentStartOffset, offset - indentStartOffset));
		indentStartOffset = offset;
	}

	bool doIndentCheck = false;
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
				doIndentCheck = true;
				++line;
				column = 0;
				indentStartOffset = offset + 1;
			}
			else if(c == '\r')
			{
				indentStartOffset = offset + 1;
			}
			else if(c != '\t' && c != ' ')
			{
				Token tok(sprawl::StringLiteral(ptr + offset, 1), line, column);
				LogError("Invalid character", TokenList(&tok, 1));
				return false;
			}
			++offset;
			continue;
		}

		if(m_significantWhitespace && doIndentCheck)
		{
			doIndentCheck = false;
			sprawl::StringLiteral lit(ptr + indentStartOffset, offset - indentStartOffset);
			indentStartOffset = offset;
			sprawl::StringLiteral& previousIndent = indents.back();

			if(memcmp(previousIndent.GetPtr(), lit.GetPtr(), previousIndent.GetLength() < lit.GetLength() ? previousIndent.GetLength() : lit.GetLength()) != 0)
			{
				Token tok(lit, line, 0);
				LogError("Non-matching indent", TokenList(&tok, 1));
				return false;
			}

			if(lit.GetLength() < previousIndent.GetLength())
			{
				indents.pop_back();
				if(indents.empty())
				{
					Token tok(lit, line, 0);
					LogError("Non-matching indent", TokenList(&tok, 1));
					return false;
				}
				Token tok(sprawl::StringLiteral("\r", 1), line, column);
				tokens.PushBack(tok);
			}
			else if(previousIndent.GetLength() < lit.GetLength())
			{
				indents.push_back(lit);
				Token tok(sprawl::StringLiteral("\t", 1), line, column);
				tokens.PushBack(tok);
			}
		}

		for(auto& kvp : m_spans)
		{
			auto& span = kvp.Value();
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
				lineStart = -1;
				columnStart = -1;
				foundSpan = true;
				matched = true;
				break;
			}
		}
		if(!foundSpan)
		{
			for(auto& kvp : m_regexSpans)
			{
				auto& span = kvp.Value();
				int ret = span.Find(sprawl::StringRef(ptr + offset, text.GetLength() - offset));
				if(ret != -1)
				{
					sprawl::StringRef capturedToken(ptr + offset, ret);
					tokens.PushBack(Token(capturedToken, lineStart, columnStart));
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
					lineStart = -1;
					columnStart = -1;
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

		if(lineStart < 0)
		{
			lineStart = line;
			columnStart = column;
		}

		partialMatch = sprawl::StringLiteral(ptr + offset, text.GetLength() - offset);
		for(auto& kvp : m_expressionLiterals)
		{
			auto& literal = kvp.Value();
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

					offset += literal.length();
					lineStart = -1;
					columnStart = -1;
					break;
				}
			}
		}

		if(!matched)
		{
			for(auto& kvp : m_literals)
			{
				auto& literal = kvp.Value();
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

						offset += ret;
						lineStart = -1;
						columnStart = -1;
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

bool H2OCompiler::Parse(const TokenList& tokens, sprawl::String& output)
{
	int offset = 0;
	sprawl::collections::List<Match*> matches;
	while(offset < tokens.Length())
	{
		TokenList tokenList = tokens.Slice(offset, tokens.Length());
		bool found = false;
		for(auto& item : m_topLevelItems)
		{
			Match* match = item->Match(*this, tokenList);
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
			//Find the next valid match so we know what all DIDN'T match here...
			int startOffset = offset;

			++offset;
			while(offset < tokens.Length())
			{
				TokenList tokenList2 = tokens.Slice(offset, tokens.Length());
				bool found2 = false;
				for(auto& item : m_topLevelItems)
				{
					Match* match = item->Match(*this, tokenList2);
					if(match)
					{
						offset += match->End();
						match->Release();
						found2 = true;
						break;
					}
				}
				if(found2)
				{
					break;
				}
				else
				{
					++offset;
				}
			}
			TokenList errorTokens = tokens.Slice(startOffset, offset-1);

			LogError(sprawl::StringLiteral("Invalid syntax"), errorTokens);
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
	output = writer.Finish();
	return true;
}

LiteralItem& H2OCompiler::AddLiteral(const sprawl::String& name, const sprawl::StringLiteral& regex, const LiteralItem::Translator& translator)
{
	auto it = m_literals.insert(LiteralItem(name, regex, translator), name);
	m_allItems.insert(&it.Value(), name);
	return it.Value();
}

SpanItem& H2OCompiler::AddSpan(const sprawl::String& name, sprawl::StringLiteral const& startLiteral, sprawl::StringLiteral const& endLiteral, const SpanItem::Translator& translator)
{
	auto it = m_spans.insert(SpanItem(name, startLiteral, endLiteral, translator), name);
	m_allItems.insert(&it.Value(), name);
	return it.Value();
}

RegexSpanItem& H2OCompiler::AddRegexSpan(const sprawl::String& name, sprawl::StringLiteral const& startRegex, sprawl::StringLiteral const& endLiteral, const RegexSpanItem::Translator& translator)
{
	auto it = m_regexSpans.insert(RegexSpanItem(name, startRegex, endLiteral, translator), name);
	m_allItems.insert(&it.Value(), name);
	return it.Value();
}

ExpressionItem& H2OCompiler::AddExpression(const sprawl::String& name, std::initializer_list<const char* const> values, const ExpressionItem::Translator& translator)
{
	StringList valueList;

	for(auto value : values)
	{
		valueList.PushBack(sprawl::StringLiteral(value, strlen(value)));
		if(value[0] != '$')
		{
			m_expressionLiterals.insert(value);
		}
	}

	auto it = m_expressions.insert(ExpressionItem(name, std::move(valueList), translator), name);
	m_allItems.insert(&it.Value(), name);
	return it.Value();
}

ExpressionItem& H2OCompiler::AddExpression(const sprawl::String& name, StringList&& valueList, const ExpressionItem::Translator& translator)
{
	for(int i = 0; i < valueList.Length(); ++i)
	{
		if(valueList[i].GetPtr()[0] != '$')
		{
			m_expressionLiterals.insert(valueList[i]);
		}
	}

	auto it = m_expressions.insert(ExpressionItem(name, std::move(valueList), translator), name);
	m_allItems.insert(&it.Value(), name);
	return it.Value();
}


GroupItem& H2OCompiler::AddGroup(const sprawl::String& name, std::initializer_list<const char* const> names)
{
	sprawl::collections::List<sprawl::StringLiteral> nameList;

	for(auto name : names)
	{
		nameList.PushBack(sprawl::StringLiteral(name, strlen(name)));
	}

	auto it = m_groups.insert(GroupItem(name, std::move(nameList)), name);
	m_allItems.insert(&it.Value(), name);
	return it.Value();
}

GroupItem& H2OCompiler::AddGroup(const sprawl::String& name, sprawl::collections::List<sprawl::StringLiteral>&& nameList)
{
	auto it = m_groups.insert(GroupItem(name, std::move(nameList)), name);
	m_allItems.insert(&it.Value(), name);
	return it.Value();
}

void H2OCompiler::AddTopLevelItem(const sprawl::String& itemName)
{
	m_topLevelItems.PushFront(m_allItems.get(itemName));
}

void H2OCompiler::AddLiteralTranslator(const sprawl::String& name, const LiteralItem::Translator& translator)
{
	m_literalTranslators.insert(translator, name);
}

void H2OCompiler::AddExpressionTranslator(const sprawl::String& name, const ExpressionItem::Translator& translator)
{
	m_expressionTranslators.insert(translator, name);
}

void H2OCompiler::AddSpanTranslator(const sprawl::String& name, const SpanItem::Translator& translator)
{
	m_spanTranslators.insert(translator, name);
}

LiteralItem::Translator H2OCompiler::GetLiteralTranslator(const sprawl::String& name)
{
	return m_literalTranslators.get(name);
}

ExpressionItem::Translator H2OCompiler::GetExpressionTranslator(const sprawl::String& name)
{
	return m_expressionTranslators.get(name);
}

SpanItem::Translator H2OCompiler::GetSpanTranslator(const sprawl::String& name)
{
	return m_spanTranslators.get(name);
}
