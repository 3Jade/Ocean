#include "Token.hpp"


Token::Token(sprawl::StringLiteral const& text, int lineNo, int column)
	: m_text(text)
	, m_lineNo(lineNo)
	, m_column(column)
{
	// NOP
}
