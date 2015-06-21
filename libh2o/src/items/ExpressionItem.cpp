#include "ExpressionItem.hpp"
#include "../matches/ExpressionMatch.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"
#include "../H2OCompiler.hpp"

/*virtual*/ ExpressionItem::ExpressionItem(const sprawl::String& name, StringList&& values, Translator const& translator)
	: Item(name)
	, m_values(std::move(values))
	, m_literals()
	, m_translator(translator)
{
	for(int i = 0; i < m_values.Length(); ++i)
	{
		if(m_values[i].GetPtr()[0] != '$')
		{
			m_literals.PushFront(i);
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

sprawl::String tabs = "";

sprawl::String PrintTokens(TokenList const& tokens)
{
	sprawl::String out = "";
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

sprawl::String PrintStringList(StringList const& tokens)
{
	sprawl::String out = "";
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
	if(tokens.Length() < m_literals.Size())
	{
		return nullptr;
	}

	if(m_values[0].GetPtr()[0] != '$')
	{
		sprawl::StringLiteral const& tok = tokens[0].Text();
		sprawl::StringLiteral const& literal = m_values[0];
		if(tok.GetLength() != literal.GetLength() || memcmp(tok.GetPtr(), literal.GetPtr(), tok.GetLength()) != 0)
		{
			return nullptr;
		}
	}

	sprawl::String oldTabs = tabs;
	tabs = tabs + "\t";

	int startLocation = -1;
	int endLocation = -1;
	int prev = -1;
	ExpressionMatch::MatchMap matches;

	struct Helper
	{
		Helper(ExpressionMatch::MatchMap& matches) : m_matches(matches) {}
		~Helper()
		{
			for(auto& vect : m_matches)
			{
				for(auto& subvect : vect.Value())
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
		sprawl::StringLiteral literal = m_values[literalIndex];
		//printf("%s%s %s\n", tabs.c_str(), GetName().c_str(), literal.GetPtr());
		if(prev + 1 < literalIndex)
		{
			int numTokens = tokens.Length();
			bool matched = false;
			for(int i = numTokens - 1; i >= 0; --i)
			{
				sprawl::StringLiteral token = tokens[i].Text();
				LoopAction loopAction = LoopAction::Continue;
				if(token.GetLength() == literal.GetLength() && !memcmp(token.GetPtr(), literal.GetPtr(), token.GetLength()))
				{
					StringList const expressionsBetween = m_values.Slice(prev + 1, literalIndex);
					endLocation = i;
					int numExpressions = expressionsBetween.Length();
					for(int expressionIndex = 0; expressionIndex < numExpressions; ++expressionIndex)
					{
						char const* expNamePtr = expressionsBetween[expressionIndex].GetPtr() + 1;
						int expNameLen = expressionsBetween[expressionIndex].GetLength() - 1;
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

						sprawl::StringLiteral storeName(expNamePtr, expNameLen);
						if(expNamePtr[expNameLen-1] == '>')
						{
							for(int idx = expNameLen - 2; idx >= 0; --idx)
							{
								if(expNamePtr[idx] == '<')
								{
									storeName = sprawl::StringLiteral(expNamePtr + idx + 1, expNameLen - idx - 2);
									expNameLen = idx;
								}
							}
						}

						sprawl::String storeNameStr(storeName);
						auto it = matches.find(storeNameStr);
						if(!it.Valid())
						{
							it = matches.insert(std::vector<std::vector< ::Match*>>(), storeNameStr);
						}
						it.Value().push_back(std::vector< ::Match*>());

						sprawl::String expNameStr(sprawl::StringLiteral(expNamePtr, expNameLen));
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

								it.Value().back().push_back(match);
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
							matches.get(storeNameStr).pop_back();
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
		else if(startLocation < int(tokens.Length() - 1))
		{
			sprawl::StringLiteral tok = tokens[startLocation+1].Text();
			if(tok.GetLength() == literal.GetLength() && !memcmp(tok.GetPtr(), literal.GetPtr(), tok.GetLength()))
			{
				//printf("%s%s Found token %.*s\n", tabs.c_str(), GetName().c_str(), (int)literal.GetLength(), literal.GetPtr());
				endLocation = startLocation + 1;
			}
			else
			{
				//printf("%s%s Couldn't match token %.*s\n", tabs.c_str(), GetName().c_str(), (int)literal.GetLength(), literal.GetPtr());
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
			char const* expNamePtr = expressionsAfter[expressionIndex].GetPtr() + 1;
			int expNameLen = expressionsAfter[expressionIndex].GetLength() - 1;
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

			sprawl::StringLiteral storeName(expNamePtr, expNameLen);
			if(expNamePtr[expNameLen-1] == '>')
			{
				for(int idx = expNameLen - 2; idx >= 0; --idx)
				{
					if(expNamePtr[idx] == '<')
					{
						storeName = sprawl::StringLiteral(expNamePtr + idx + 1, expNameLen - idx - 2);
						expNameLen = idx;
					}
				}
			}

			sprawl::String storeNameStr(storeName);
			auto it = matches.find(storeNameStr);
			if(!it.Valid())
			{
				it = matches.insert(std::vector<std::vector< ::Match*>>(), storeNameStr);
			}
			it.Value().push_back(std::vector< ::Match*>());

			sprawl::String expNameStr(sprawl::StringLiteral(expNamePtr, expNameLen));
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
					it.Value().back().push_back(match);
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
			for(auto& subvect : vect.Value())
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
