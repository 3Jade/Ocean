#include "RegexSpanItem.hpp"
#include "../matches/StringMatch.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"
#include "../H2OCompiler.hpp"

RegexSpanItem::RegexSpanItem(std::string_view const& name, std::string_view const& startRegex, std::string_view const& endLiteral, Translator const& translator)
	: Item(name)
	, m_startRegex(re2::StringPiece(startRegex.data(), startRegex.length()))
	, m_startRegexBase(startRegex)
	, m_end(endLiteral)
	, m_translator(translator)
	, m_escape(nullptr, 0)
	, m_restrictNewlines(false)
	, m_startHint(nullptr, 0)
{
	int hintLength = 0;
	char const* const startRegexPtr = startRegex.data();
	while(hintLength < startRegex.length() && (isalpha(startRegexPtr[hintLength]) || isdigit(startRegexPtr[hintLength])))
	{
		++hintLength;
	}
	if(hintLength != 0)
	{
		m_startHint = std::string_view(startRegexPtr, hintLength);
	}
}

RegexSpanItem::RegexSpanItem(const RegexSpanItem& other)
	: Item(other)
	, m_startRegex(re2::StringPiece(other.m_startRegexBase.data(), other.m_startRegexBase.length()))
	, m_startRegexBase(other.m_startRegexBase)
	, m_end(other.m_end)
	, m_translator(other.m_translator)
	, m_escape(other.m_escape)
	, m_restrictNewlines(other.m_restrictNewlines)
	, m_startHint(other.m_startHint)
{
	// NOP
}

int RegexSpanItem::_find(std::string_view const& text, int& startPosition, int& endPosition)
{
	if(m_startHint.length() != 0)
	{
		if(m_startHint.length() > text.length())
		{
			return -1;
		}
		if(memcmp(m_startHint.data(), text.data(), m_startHint.length()) != 0)
		{
			return -1;
		}
	}
	re2::StringPiece piece(text.data(), text.length());

	int nArgs = m_startRegex.NumberOfCapturingGroups() + 1;
	RE2::Arg* args = (RE2::Arg*)alloca(sizeof(RE2::Arg) * (nArgs-1));
	re2::StringPiece* pieces = new re2::StringPiece[nArgs];
	pieces[0] = piece;

	for(int i = 0; i < nArgs-1; ++i)
	{
		args[i] = RE2::Arg(&pieces[i+1]);
	}

	if(RE2::ConsumeN(&piece, m_startRegex, &args, nArgs-1))
	{
		std::string end;
		m_startRegex.Rewrite(&end, re2::StringPiece(m_end.data(), m_end.length()), pieces, nArgs);
		std::string escape;
		if(m_escape.length())
		{
			m_startRegex.Rewrite(&escape, m_escape.data(), pieces, nArgs);
		}
		free(pieces);
		startPosition = text.length() - piece.length();

		while(!piece.empty())
		{
			if(end.length() == 0 && (piece[0] == '\n' || piece[0] == '\r'))
			{
				endPosition = text.length() - piece.length();
				return endPosition;
			}
			if(m_restrictNewlines && piece[0] == '\n')
			{
				return -1;
			}
			if(!escape.empty() && piece.length() >= escape.length() && !memcmp(piece.data(), escape.c_str(), escape.length()))
			{
				piece.set(piece.data() + escape.length(), piece.length() - escape.length());
				continue;
			}
			if(piece.length() >= end.length() && !memcmp(piece.data(), end.c_str(), end.length()))
			{
				endPosition = text.length() - piece.length();
				return endPosition + end.length();
			}
			piece.set(piece.data() + 1, piece.length() - 1);
		}
		if(end.length() == 0)
		{
			endPosition = text.length();
			return text.length();
		}
		return -1;
	}
	delete[] pieces;
	return -1;
}

int RegexSpanItem::Find(std::string_view const& text)
{
	int start;
	int end;
	return _find(text, start, end);
}

Match* RegexSpanItem::Match(H2OCompiler const& /*compiler*/, TokenList const& tokens)
{
	std::string_view const& lit = tokens[0].Text();
	int start;
	int end;
	int ret = _find(lit, start, end);
	if(ret != -1)
	{
		return StringMatch::Create(*this, tokens.Slice(0, 1), std::string_view(lit.data() + start, end - start));
	}
	return nullptr;
}

bool RegexSpanItem::Translate(BytecodeWriter& writer, ::Match const& match)
{
	StringMatch const& stringMatch = static_cast<StringMatch const&>(match);
	if(m_translator)
	{
		OceanValue value;
		if(!m_translator(writer, stringMatch, value))
		{
			return false;
		}
		writer.Stack_Push(value);
	}
	return true;
}
