#pragma once

#include <sprawl/string/String.hpp>

class Token
{
public:
	Token(sprawl::StringLiteral const& text, int lineNo, int column);

	sprawl::StringLiteral Text() const { return m_text; }
	int LineNo() const { return m_lineNo; }
	int Column() const { return m_column; }
private:
	sprawl::StringLiteral m_text;
	int m_lineNo;
	int m_column;
};
