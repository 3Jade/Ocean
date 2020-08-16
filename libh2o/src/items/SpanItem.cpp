#include "SpanItem.hpp"
#include "../matches/StringMatch.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"
#include "../H2OCompiler.hpp"

SpanItem::SpanItem(std::string_view const& name, std::string_view const& startLiteral, std::string_view const& endLiteral, Translator const& translator)
	: Item(name)
	, m_start(startLiteral)
	, m_end(endLiteral)
	, m_translator(translator)
	, m_escape(nullptr, 0)
	, m_restrictNewlines(false)
{
	// NOP
}

SpanItem::SpanItem(const SpanItem& other)
	: Item(other)
	, m_start(other.m_start)
	, m_end(other.m_end)
	, m_translator(other.m_translator)
	, m_escape(other.m_escape)
	, m_restrictNewlines(other.m_restrictNewlines)
{
	// NOP
}

int SpanItem::_find(std::string_view const& text, int& startPosition, int& endPosition)
{
	if(text.length() >= m_start.length() && !memcmp(text.data(), m_start.data(), m_start.length()))
	{
		startPosition = m_start.length();
		int offset = m_start.length();
		int const length = text.length();
		char const* const ptr = text.data();

		while(offset < length)
		{
			if(m_end.length() == 0 && (ptr[offset] == '\n' || ptr[offset] == '\r'))
			{
				endPosition = offset;
				return offset;
			}
			if(m_restrictNewlines && ptr[offset] == '\n')
			{
				return -1;
			}
			if(m_escape.length() != 0 && (length - offset) >= m_escape.length() && !memcmp(ptr + offset, m_escape.data(), m_escape.length()))
			{
				offset += m_escape.length();
				continue;
			}
			if(m_end.length() != 0 && (length - offset) >= m_end.length() && !memcmp(ptr + offset, m_end.data(), m_end.length()))
			{
				endPosition = offset;
				return offset + m_end.length();
			}
			++offset;
		}
		if(m_end.length() == 0)
		{
			endPosition = length;
			return length;
		}
		return -1;
	}
	return -1;
}

int SpanItem::Find(std::string_view const& text)
{
	int start;
	int end;
	return _find(text, start, end);
}

Match* SpanItem::Match(H2OCompiler const& /*compiler*/, TokenList const& tokens)
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

bool SpanItem::Translate(BytecodeWriter& writer, ::Match const& match)
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
