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

bool H2OCompiler::Tokenize(std::string_view const& text, TokenList& tokens)
{
	std::string_view partialMatch("", 0);

	int lineStart = -1;
	int columnStart = -1;
	size_t offset = 0;
	size_t length = text.length();
	size_t line = 0;
	size_t column = 0;

	bool matched = false;
	char const* const ptr = text.data();

	std::vector<std::string_view> indents;
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
				Token tok(std::string_view(ptr + offset, 1), line, column);
				LogError("Invalid character", TokenList(&tok, 1));
				return false;
			}
			++offset;
			c = *(ptr + offset);
		}
		indents.push_back(std::string_view(ptr + indentStartOffset, offset - indentStartOffset));
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
				Token tok(std::string_view(ptr + offset, 1), line, column);
				LogError("Invalid character", TokenList(&tok, 1));
				return false;
			}
			++offset;
			continue;
		}

		if(m_significantWhitespace && doIndentCheck)
		{
			doIndentCheck = false;
			std::string_view lit(ptr + indentStartOffset, offset - indentStartOffset);
			indentStartOffset = offset;
			std::string_view& previousIndent = indents.back();

			if(memcmp(previousIndent.data(), lit.data(), previousIndent.length() < lit.length() ? previousIndent.length() : lit.length()) != 0)
			{
				Token tok(lit, line, 0);
				LogError("Non-matching indent", TokenList(&tok, 1));
				return false;
			}

			if(lit.length() < previousIndent.length())
			{
				indents.pop_back();
				if(indents.empty())
				{
					Token tok(lit, line, 0);
					LogError("Non-matching indent", TokenList(&tok, 1));
					return false;
				}
				Token tok(std::string_view("\r", 1), line, column);
				tokens.PushBack(tok);
			}
			else if(previousIndent.length() < lit.length())
			{
				indents.push_back(lit);
				Token tok(std::string_view("\t", 1), line, column);
				tokens.PushBack(tok);
			}
		}

		for(auto& kvp : m_spans)
		{
			auto& span = *kvp.value;
			int ret = span.Find(std::string_view(ptr + offset, text.length() - offset));
			if(ret != -1)
			{
				std::string_view capturedToken(ptr + offset, ret);
				tokens.PushBack(Token(capturedToken, line, column));
				for(int i = 0; i < ret; ++i)
				{
					if(*(capturedToken.data() + i) == '\n')
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
				auto& span = *kvp.value;
				int ret = span.Find(std::string_view(ptr + offset, text.length() - offset));
				if(ret != -1)
				{
					std::string_view capturedToken(ptr + offset, ret);
					tokens.PushBack(Token(capturedToken, lineStart, columnStart));
					for(int i = 0; i < ret; ++i)
					{
						if(*(capturedToken.data() + i) == '\n')
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

		partialMatch = std::string_view(ptr + offset, text.length() - offset);
		for(auto& literal : m_expressionLiterals)
		{
			if(literal.length() > partialMatch.length())
			{
				continue;
			}
			if(!memcmp(partialMatch.data(), literal.data(), literal.length()))
			{
				char endChar = *(partialMatch.data() + literal.length() - 1);
				char nextChar = *(partialMatch.data() + literal.length());
				if(!isalnum(endChar) || !isalnum(nextChar))
				{
					matched = true;
					Token token(std::string_view(partialMatch.data(), literal.length()), lineStart, columnStart);
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
				auto& literal = *kvp.value;
				int ret = literal.Find(partialMatch);
				if(ret != -1)
				{
					char endChar = *(partialMatch.data() + ret - 1);
					char nextChar = *(partialMatch.data() + ret);
					if(!isalnum(endChar) || !isalnum(nextChar))
					{
						matched = true;
						std::string_view literalMatch(ptr + offset, ret);
						Token token(literalMatch, lineStart, columnStart);
						tokens.PushBack(token);

						for(int i = 0; i < ret; ++i)
						{
							if(*(literalMatch.data() + i) == '\n')
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
			partialMatch = std::string_view(ptr + offset, endOffset - offset);
			Token token(partialMatch, lineStart, columnStart);
			LogError(std::string("Invalid syntax: `") + std::string(partialMatch) + "`", TokenList(&token, 1));
			return false;
		}
	}

	return true;
}

bool H2OCompiler::Parse(const TokenList& tokens, std::string& output)
{
	int offset = 0;
	std::list<Match*> matches;
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
				matches.push_back(match);
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

			LogError("Invalid syntax", errorTokens);
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

LiteralItem& H2OCompiler::AddLiteral(const std::string_view& name, const std::string_view& regex, const LiteralItem::Translator& translator)
{
	auto it = m_literals.CheckedInsert(name, new LiteralItem(name, regex, translator)).iterator;
	m_allItems.Insert(name, it->value);
	return *it->value;
}

SpanItem& H2OCompiler::AddSpan(const std::string_view& name, std::string_view const& startLiteral, std::string_view const& endLiteral, const SpanItem::Translator& translator)
{
	auto it = m_spans.CheckedInsert(name, new SpanItem(name, startLiteral, endLiteral, translator)).iterator;
	m_allItems.Insert(name, it->value);
	return *it->value;
}

RegexSpanItem& H2OCompiler::AddRegexSpan(const std::string_view& name, std::string_view const& startRegex, std::string_view const& endLiteral, const RegexSpanItem::Translator& translator)
{
	auto it = m_regexSpans.CheckedInsert(name, new RegexSpanItem(name, startRegex, endLiteral, translator)).iterator;
	m_allItems.Insert(name, it->value);
	return *it->value;
}

ExpressionItem& H2OCompiler::AddExpression(const std::string_view& name, std::initializer_list<const char* const> values, const ExpressionItem::Translator& translator)
{
	StringList valueList;

	for(auto value : values)
	{
		valueList.PushBack(std::string_view(value, strlen(value)));
		if(value[0] != '$')
		{
			m_expressionLiterals.insert(value);
		}
	}

	auto it = m_expressions.CheckedInsert(name, new ExpressionItem(name, std::move(valueList), translator)).iterator;
	m_allItems.Insert(name, it->value);
	return *it->value;
}

ExpressionItem& H2OCompiler::AddExpression(const std::string_view& name, StringList&& valueList, const ExpressionItem::Translator& translator)
{
	for(int i = 0; i < valueList.Length(); ++i)
	{
		if(valueList[i].data()[0] != '$')
		{
			m_expressionLiterals.insert(valueList[i]);
		}
	}

	auto it = m_expressions.CheckedInsert(name, new ExpressionItem(name, std::move(valueList), translator)).iterator;
	m_allItems.Insert(name, it->value);
	return *it->value;
}


GroupItem& H2OCompiler::AddGroup(const std::string_view& name, std::initializer_list<const char* const> names)
{
	std::list<std::string_view> nameList;

	for(auto name : names)
	{
		nameList.push_back(std::string_view(name, strlen(name)));
	}

	auto it = m_groups.CheckedInsert(name, new GroupItem(name, std::move(nameList))).iterator;
	m_allItems.Insert(name, it->value);
	return *it->value;
}

GroupItem& H2OCompiler::AddGroup(const std::string_view& name, std::list<std::string_view>&& nameList)
{
	auto it = m_groups.CheckedInsert(name, new GroupItem(name, std::move(nameList))).iterator;
	m_allItems.Insert(name, it->value);
	return *it->value;
}

void H2OCompiler::AddTopLevelItem(const std::string_view& itemName)
{
	m_topLevelItems.push_back(m_allItems.Get(itemName));
}

void H2OCompiler::AddLiteralTranslator(const std::string_view& name, const LiteralItem::Translator& translator)
{
	m_literalTranslators.Insert(name, translator);
}

void H2OCompiler::AddExpressionTranslator(const std::string_view& name, const ExpressionItem::Translator& translator)
{
	m_expressionTranslators.Insert(name, translator);
}

void H2OCompiler::AddSpanTranslator(const std::string_view& name, const SpanItem::Translator& translator)
{
	m_spanTranslators.Insert(name, translator);
}

LiteralItem::Translator H2OCompiler::GetLiteralTranslator(const std::string_view& name)
{
	return m_literalTranslators.Get(name);
}

ExpressionItem::Translator H2OCompiler::GetExpressionTranslator(const std::string_view& name)
{
	return m_expressionTranslators.Get(name);
}

SpanItem::Translator H2OCompiler::GetSpanTranslator(const std::string_view& name)
{
	return m_spanTranslators.Get(name);
}
