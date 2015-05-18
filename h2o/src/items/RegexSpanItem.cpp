#include "RegexSpanItem.hpp"
#include "../matches/StringMatch.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"
#include "../H2OCompiler.hpp"

RegexSpanItem::RegexSpanItem(const sprawl::String& name, sprawl::StringLiteral const& startRegex, sprawl::StringLiteral const& endLiteral, Translator const& translator)
	: Item(name)
	, m_startRegex(re2::StringPiece(startRegex.GetPtr(), startRegex.GetLength()))
	, m_startRegexBase(startRegex)
	, m_end(endLiteral)
	, m_translator(translator)
	, m_escape(nullptr, 0)
	, m_restrictNewlines(false)
	, m_startHint(nullptr, 0)
{
	int hintLength = 0;
	char const* const startRegexPtr = startRegex.GetPtr();
	while(hintLength < startRegex.GetLength() && (isalpha(startRegexPtr[hintLength]) || isdigit(startRegexPtr[hintLength])))
	{
		++hintLength;
	}
	if(hintLength != 0)
	{
		m_startHint = sprawl::StringLiteral(startRegexPtr, hintLength);
	}
}

RegexSpanItem::RegexSpanItem(const RegexSpanItem& other)
	: Item(other)
	, m_startRegex(re2::StringPiece(other.m_startRegexBase.GetPtr(), other.m_startRegexBase.GetLength()))
	, m_startRegexBase(other.m_startRegexBase)
	, m_end(other.m_end)
	, m_translator(other.m_translator)
	, m_escape(other.m_escape)
	, m_restrictNewlines(other.m_restrictNewlines)
	, m_startHint(other.m_startHint)
{
	// NOP
}

int RegexSpanItem::_find(sprawl::StringLiteral const& text, int& startPosition, int& endPosition)
{
	if(m_startHint.GetLength() != 0)
	{
		if(m_startHint.GetLength() > text.GetLength())
		{
			return -1;
		}
		if(memcmp(m_startHint.GetPtr(), text.GetPtr(), m_startHint.GetLength()) != 0)
		{
			return -1;
		}
	}
	re2::StringPiece piece(text.GetPtr(), text.GetLength());

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
		m_startRegex.Rewrite(&end, re2::StringPiece(m_end.GetPtr(), m_end.GetLength()), pieces, nArgs);
		std::string escape;
		if(m_escape.GetLength())
		{
			m_startRegex.Rewrite(&escape, m_escape.GetPtr(), pieces, nArgs);
		}
		free(pieces);
		startPosition = text.GetLength() - piece.length();

		while(!piece.empty())
		{
			if(end.length() == 0 && (piece[0] == '\n' || piece[0] == '\r'))
			{
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
				endPosition = text.GetLength() - piece.length();
				return endPosition + end.length();
			}
			piece.set(piece.data() + 1, piece.length() - 1);
		}
		if(end.length() == 0)
		{
			endPosition = text.GetLength();
			return text.GetLength();
		}
		return -1;
	}
	delete[] pieces;
	return -1;
}

int RegexSpanItem::Find(sprawl::StringLiteral const& text)
{
	int start;
	int end;
	return _find(text, start, end);
}

Match* RegexSpanItem::Match(H2OCompiler const& /*compiler*/, TokenList const& tokens)
{
	sprawl::StringLiteral const& lit = tokens[0].Text();
	int start;
	int end;
	int ret = _find(lit, start, end);
	if(ret != -1)
	{
		return StringMatch::Create(*this, tokens.Slice(0, 1), sprawl::StringRef(lit.GetPtr() + start, end - start));
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
