#include "ExpressionItem.hpp"
#include "../matches/ExpressionMatch.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"
#include "../H2OCompiler.hpp"

/*virtual*/ ExpressionItem::ExpressionItem(std::string_view const& name, StringList&& values, Translator const& translator)
	: Item(name)
	, m_values(std::move(values))
	, m_literals()
	, m_translator(translator)
{
	for(int i = 0; i < m_values.Length(); ++i)
	{
		if(m_values[i].data()[0] != '$')
		{
			m_literals.push_back(i);
		}
	}
}

enum RepeatType : char
{
	One = '\0',
	ZeroOrOne = '?',
	ZeroOrMore = '*',
	OneOrMore = '+',
	Zero = '!',
};

enum class LoopAction
{
	Break = 0,
	Continue = 1
};

std::string tabs = "";

std::string PrintTokens(TokenList const& tokens)
{
	std::string out = "";
	bool first = true;
	out += "[";
	for(int i = 0; i < tokens.Length(); ++i)
	{
		if(!first)
		{
			out += ",";
		}
		out += " ";
		out += tokens[i].Text();
	}
	out += "]";
	return out;
}

std::string PrintStringList(StringList const& tokens)
{
	std::string out = "";
	bool first = true;
	out += "[";
	for(int i = 0; i < tokens.Length(); ++i)
	{
		if(!first)
		{
			out += ",";
		}
		out += " ";
		out += tokens[i];
	}
	out += "]";
	return out;
}

Match* ExpressionItem::Match(H2OCompiler const& compiler, const TokenList& tokens) /*override*/
{
	if(tokens.Length() < m_literals.size())
	{
		return nullptr;
	}

	if(m_values[0].data()[0] != '$')
	{
		std::string_view const& tok = tokens[0].Text();
		std::string_view const& literal = m_values[0];
		if(tok.length() != literal.length() || memcmp(tok.data(), literal.data(), tok.length()) != 0)
		{
			return nullptr;
		}
	}

	std::string oldTabs = tabs;
	tabs = tabs + "\t";

	int64_t startLocation = -1;
	int64_t endLocation = -1;
	int64_t prev = -1;
	ExpressionMatch::MatchMap matches;

	struct Helper
	{
		Helper(ExpressionMatch::MatchMap& matches) : m_matches(matches) {}
		~Helper()
		{
			for(auto& vect : m_matches)
			{
				for(auto& subvect : vect.value)
				{
					for(auto& item : subvect)
					{
						item->Release();
					}
				}
			}
		}
		ExpressionMatch::MatchMap& m_matches;
	};

	//printf("%s%s: Matching against %s\n", tabs.c_str(), GetName().c_str(), PrintTokens(tokens).c_str());
	for(auto literalIndex : m_literals)
	{
		std::string_view literal = m_values[literalIndex];
		//printf("%s%s %s\n", tabs.c_str(), GetName().c_str(), literal.data());
		if(prev + 1 < literalIndex)
		{
			int numTokens = tokens.Length();
			bool matched = false;
			for(int i = numTokens - 1; i >= 0; --i)
			{
				std::string_view token = tokens[i].Text();
				LoopAction loopAction = LoopAction::Continue;
				if(token.length() == literal.length() && !memcmp(token.data(), literal.data(), token.length()))
				{
					StringList const expressionsBetween = m_values.Slice(prev + 1, literalIndex);
					endLocation = i;
					int numExpressions = expressionsBetween.Length();
					for(int expressionIndex = 0; expressionIndex < numExpressions; ++expressionIndex)
					{
						char const* expNamePtr = expressionsBetween[expressionIndex].data() + 1;
						int64_t expNameLen = expressionsBetween[expressionIndex].length() - 1;
						RepeatType repeat = RepeatType::One;
						switch(expNamePtr[0])
						{
							case RepeatType::ZeroOrOne:
							case RepeatType::ZeroOrMore:
							case RepeatType::OneOrMore:
							case RepeatType::Zero:
								repeat = RepeatType(expNamePtr[0]);
								++expNamePtr;
								--expNameLen;
								break;
							default:
								break;
						}

						std::string_view storeName(expNamePtr, expNameLen);
						if(expNamePtr[expNameLen-1] == '>')
						{
							for(int idx = expNameLen - 2; idx >= 0; --idx)
							{
								if(expNamePtr[idx] == '<')
								{
									storeName = std::string_view(expNamePtr + idx + 1, expNameLen - idx - 2);
									expNameLen = idx;
								}
							}
						}

						auto it = matches.find(storeName);
						if(!it.Valid())
						{
							it = matches.CheckedInsert(storeName, std::vector<std::vector< ::Match*>>()).iterator;
						}
						it->value.push_back(std::vector< ::Match*>());

						std::string_view expNameStr(expNamePtr, expNameLen);
						Item* expReferencedItem = compiler.GetItem(expNameStr);

						int start = startLocation + 1;
						bool found = false;

						//printf("%s%s start %d length %zd\n", tabs.c_str(), GetName().c_str(), start, tokens.Length());

						for(;;)
						{
							//printf("%s%s start start end %d %d %d\n", tabs.c_str(), GetName().c_str(), start, startLocation, endLocation);
							TokenList tokenList = tokens.Slice(start, endLocation);
							::Match* match = expReferencedItem->Match(compiler, tokenList);
							//printf("%s%s %s %p %zd %s %zd\n", tabs.c_str(), GetName().c_str(), expNameStr.c_str(), match, match ? match->End() : -1, PrintTokens(tokenList).c_str(), tokenList.Length());
							if(match)
							{
								if(repeat == RepeatType::Zero)
								{
									match->Release();
									loopAction = LoopAction::Continue;
									break;
								}

								it->value.back().push_back(match);
								loopAction = LoopAction::Break;
								found = true;

								if(repeat == RepeatType::One || repeat == RepeatType::ZeroOrOne)
								{
									if(match->End() != tokenList.Length())
									{
										//printf("%s%s Not filling.\n", tabs.c_str(), GetName().c_str());
										loopAction = LoopAction::Continue;
									}
									break;
								}
								start += match->End();
							}
							else
							{
								if(tokenList.Length() != 0)
								{
									//printf("%s%s Found but not filling.\n", tabs.c_str(), GetName().c_str());
									loopAction = LoopAction::Continue;
								}
								else if(repeat == RepeatType::ZeroOrMore || repeat == RepeatType::ZeroOrOne || found)
								{
									//printf("%s%s Found, or zero repeat allowed.\n", tabs.c_str(), GetName().c_str());
									loopAction = LoopAction::Break;
								}
								else
								{
									//printf("%s%s Not found.\n", tabs.c_str(), GetName().c_str());
									loopAction = LoopAction::Continue;
								}
								break;
							}
						}
						if(loopAction == LoopAction::Continue)
						{
							matches.Get(storeName).pop_back();
							break;
						}
					}
					if(loopAction == LoopAction::Continue)
					{
						//printf("%sContinue...\n", tabs.c_str());
					}
					else
					{
						//printf("%sFound it, break!\n", tabs.c_str());
						matched = true;
						break;
					}
				}
			}
			if(!matched)
			{
				//printf("%s%s Could not match sub-expression %s\n", tabs.c_str(), GetName().c_str(), PrintStringList(m_values.Slice(prev + 1, literalIndex)).c_str());
				tabs = oldTabs;
				return nullptr;
			}
		}
		else if(startLocation < int64_t(tokens.Length()) - 1)
		{
			std::string_view tok = tokens[startLocation+1].Text();
			if(tok.length() == literal.length() && !memcmp(tok.data(), literal.data(), tok.length()))
			{
				//printf("%s%s Found token %.*s\n", tabs.c_str(), GetName().c_str(), (int)literal.length(), literal.data());
				endLocation = startLocation + 1;
			}
			else
			{
				//printf("%s%s Couldn't match token %.*s\n", tabs.c_str(), GetName().c_str(), (int)literal.length(), literal.data());
				tabs = oldTabs;
				return nullptr;
			}
		}
		else
		{
			//printf("%s%s Empty tokens list\n", tabs.c_str(), GetName().c_str());
			tabs = oldTabs;
			return nullptr;
		}

		prev = literalIndex;
		startLocation = endLocation;
	}

	++endLocation;
	//printf("%s%s endLocation %d length %zd\n", tabs.c_str(), GetName().c_str(), endLocation, tokens.Length());

	if(prev + 1 < m_values.Length())
	{
		StringList expressionsAfter = m_values.Slice(prev+1, m_values.Length());
		//printf("%s%s Expressions to end %s\n", tabs.c_str(), GetName().c_str(), PrintStringList(expressionsAfter).c_str());
		int numExpressions = expressionsAfter.Length();
		for(int expressionIndex = 0; expressionIndex < numExpressions; ++expressionIndex)
		{
			char const* expNamePtr = expressionsAfter[expressionIndex].data() + 1;
			int64_t expNameLen = expressionsAfter[expressionIndex].length() - 1;
			RepeatType repeat = RepeatType::One;
			switch(expNamePtr[0])
			{
				case RepeatType::ZeroOrOne:
				case RepeatType::ZeroOrMore:
				case RepeatType::OneOrMore:
				case RepeatType::Zero:
					repeat = RepeatType(expNamePtr[0]);
					++expNamePtr;
					--expNameLen;
					break;
				default:
					break;
			}

			std::string_view storeName(expNamePtr, expNameLen);
			if(expNamePtr[expNameLen-1] == '>')
			{
				for(int idx = expNameLen - 2; idx >= 0; --idx)
				{
					if(expNamePtr[idx] == '<')
					{
						storeName = std::string_view(expNamePtr + idx + 1, expNameLen - idx - 2);
						expNameLen = idx;
					}
				}
			}

			auto it = matches.find(storeName);
			if(!it.Valid())
			{
				it = matches.CheckedInsert(storeName, std::vector<std::vector< ::Match*>>()).iterator;
			}
			it->value.push_back(std::vector< ::Match*>());

			std::string_view expNameStr(expNamePtr, expNameLen);
			Item* expReferencedItem = compiler.GetItem(expNameStr);

			bool found = false;
			for(;;)
			{
				TokenList tokenList = tokens.Slice(endLocation, tokens.Length());
				//printf("%s%s Matching %s against %s %zd\n", tabs.c_str(), GetName().c_str(), expNameStr.c_str(), PrintTokens(tokenList).c_str(), tokenList.Length());
				::Match* match = expReferencedItem->Match(compiler, tokenList);
				//printf("%s%s Matched  %p %zd %d\n", tabs.c_str(), GetName().c_str(), match, match ? match->End() : -1, endLocation);
				if(match)
				{
					if(repeat == RepeatType::Zero)
					{
						match->Release();
						tabs = oldTabs;
						return nullptr;
					}

					endLocation += match->End();
					it->value.back().push_back(match);
					found = true;
					if(repeat == RepeatType::One || repeat == RepeatType::ZeroOrOne)
					{
						break;
					}
				}
				else
				{
					if(repeat != RepeatType::ZeroOrMore && repeat != RepeatType::ZeroOrOne && !found)
					{
						//printf("%s%s Could not match ending sub-expression : %s\n", tabs.c_str(), GetName().c_str(), PrintStringList(expressionsAfter).c_str());
						tabs = oldTabs;
						return nullptr;
					}
					break;
				}
			}
		}
	}

	//printf("%s%s tokens %s\n", tabs.c_str(), GetName().c_str(), PrintTokens(tokens).c_str());
	//printf("%s%s endLocation %d\n", tabs.c_str(), GetName().c_str(), endLocation);
	//printf("%s%s Found matching expression: %s\n", tabs.c_str(), GetName().c_str(), PrintTokens(tokens.Slice(0, endLocation)).c_str());

	for(auto it = matches.begin(); it != matches.end(); ++it)
	{
		//printf("%s\n", it.Key().c_str());
	}

	tabs = oldTabs;
	return ExpressionMatch::Create(*this, tokens.Slice(0, endLocation), std::move(matches));
}

bool ExpressionItem::Translate(BytecodeWriter& writer, ::Match const& match)
{
	ExpressionMatch const& expressionMatch = static_cast<ExpressionMatch const&>(match);
	if(m_translator)
	{
		return m_translator(writer, expressionMatch);
	}
	else
	{
		for(auto& vect : expressionMatch)
		{
			for(auto& subvect : vect.value)
			{
				for(auto& match : subvect)
				{
					if(!match->Translate(writer))
					{
						return false;
					}
				}
			}
		}
		return true;
	}
}
