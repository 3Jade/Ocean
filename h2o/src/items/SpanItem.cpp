#include "SpanItem.hpp"
#include "../matches/StringMatch.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"

SpanItem::SpanItem(const sprawl::String& name, const char* const startLiteral, const char* const endLiteral, Translator const& translator)
	: Item(name)
	, m_start(startLiteral, strlen(startLiteral))
	, m_end(endLiteral, strlen(endLiteral))
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

int SpanItem::_find(sprawl::StringLiteral const& text, int& startPosition, int& endPosition)
{
	if(text.GetLength() >= m_start.GetLength() && !memcmp(text.GetPtr(), m_start.GetPtr(), m_start.GetLength()))
	{
		startPosition = m_start.GetLength();
		int offset = m_start.GetLength();
		int const length = text.GetLength();
		char const* const ptr = text.GetPtr();

		while(offset < length)
		{
			if(m_restrictNewlines && ptr[offset] == '\n')
			{
				return -1;
			}
			if(m_escape.GetLength() != 0 && (length - offset) >= m_escape.GetLength() && !memcmp(ptr + offset, m_escape.GetPtr(), m_escape.GetLength()))
			{
				offset += m_escape.GetLength();
				continue;
			}
			if(m_end.GetLength() != 0 && (length - offset) >= m_end.GetLength() && !memcmp(ptr + offset, m_end.GetPtr(), m_end.GetLength()))
			{
				endPosition = offset;
				return offset + m_end.GetLength();
			}
			++offset;
		}
		printf("End\n");
		return -1;
	}
	return -1;
}

int SpanItem::Find(sprawl::StringLiteral const& text)
{
	int start;
	int end;
	return _find(text, start, end);
}

Match* SpanItem::Match(TokenList const& tokens)
{
	sprawl::StringLiteral const& lit = tokens[0].Text();
	int start;
	int end;
	int ret = _find(lit, start, end);
	if(ret != -1)
	{
		return StringMatch::Create(*this, 1, sprawl::StringRef(lit.GetPtr() + start, lit.GetLength() - start - end));
	}
	return nullptr;
}

bool SpanItem::Translate(BytecodeWriter& writer, ::Match const& match)
{
	StringMatch const& stringMatch = static_cast<StringMatch const&>(match);
	if(m_translator)
	{
		int64_t value;
		if(!m_translator(writer, stringMatch, value))
		{
			return false;
		}
		writer.Stack_Push(value);
	}
	else
	{
		writer.Stack_Push(stringMatch.GetString());
	}
	return true;
}

/*static*/ sprawl::collections::HashMap<SpanItem, sprawl::KeyAccessor<SpanItem, sprawl::String>> SpanItem::ms_spans;