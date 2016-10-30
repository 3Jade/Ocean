#include "LiteralItem.hpp"
#include <sprawl/memory/PoolAllocator.hpp>
#include "../matches/RegexMatch.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"
#include "../H2OCompiler.hpp"

LiteralItem::LiteralItem(sprawl::String const& name, sprawl::StringLiteral regex, Translator const& translator)
	: Item(name)
	, m_regex(re2::StringPiece(regex.GetPtr(), regex.GetLength()))
	, m_regexBase(regex)
	, m_translator(translator)
	, m_startHint(nullptr, 0)
{
	int hintLength = 0;
	char const* const regexPtr = regex.GetPtr();
	while(hintLength < regex.GetLength() && (isalpha(regexPtr[hintLength]) || isdigit(regexPtr[hintLength])))
	{
		++hintLength;
	}
	if(hintLength != 0)
	{
		m_startHint = sprawl::StringLiteral(regexPtr, hintLength);
	}
}

LiteralItem::LiteralItem(LiteralItem const& other)
	: Item(other)
	, m_regex(re2::StringPiece(other.m_regexBase.GetPtr(), other.m_regexBase.GetLength()))
	, m_regexBase(other.m_regexBase)
	, m_translator(other.m_translator)
	, m_startHint(other.m_startHint)
{

}

size_t LiteralItem::GroupNameToIndex(sprawl::String const& name)
{
	return m_regex.NamedCapturingGroups().at(name.toStdString());
}

int LiteralItem::Find(sprawl::StringLiteral const& text)
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
	if(RE2::ConsumeN(&piece, m_regex, nullptr, 0))
	{
		return text.GetLength() - piece.length();
	}
	return -1;
}

/*virtual*/ Match* LiteralItem::Match(H2OCompiler const& /*compiler*/, TokenList const& tokens) /*override*/
{
	Token const& token = tokens[0];
	int nArgs = m_regex.NumberOfCapturingGroups();
	RE2::Arg* args = (RE2::Arg*)alloca(sizeof(RE2::Arg) * nArgs);
	re2::StringPiece* pieces = new re2::StringPiece[nArgs + 1];
	pieces[0] = re2::StringPiece(token.Text().GetPtr(), token.Text().GetLength());
	for(int i = 0; i < nArgs; ++i)
	{
		args[i] = RE2::Arg(&pieces[i+1]);
	}

	if(RE2::FullMatchN(pieces[0], m_regex, &args, nArgs))
	{
		return RegexMatch::Create(*this, tokens.Slice(0, 1), pieces);
	}
	else
	{
		delete[] pieces;
		return nullptr;
	}
}

bool LiteralItem::Translate(BytecodeWriter& writer, ::Match const& match)
{
	RegexMatch const& rMatch = static_cast<RegexMatch const&>(match);
	if(m_translator)
	{
		OceanValue value;
		if(!m_translator(writer, rMatch, value))
		{
			return false;
		}
		writer.Stack_Push(value);
	}
	else
	{
		writer.Variable_Load(writer.GetStringOffset(rMatch.Group(0)));
	}
	return true;
}

bool LiteralItem::GetLiteralValue(BytecodeWriter const& writer, RegexMatch const& match, OceanValue& outValue)
{
	RegexMatch const& rMatch = static_cast<RegexMatch const&>(match);
	if(m_translator)
	{
		if(!m_translator(writer, rMatch, outValue))
		{
			return false;
		}
	}
	else
	{
		outValue = writer.GetStringOffset(match.Group(0));
	}
	return true;
}
