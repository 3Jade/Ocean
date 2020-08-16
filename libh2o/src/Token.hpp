#pragma once

#include <string_view>

class Token
{
public:
	Token(std::string_view const& text, int lineNo, int column);

	std::string_view Text() const { return m_text; }
	int LineNo() const { return m_lineNo; }
	int Column() const { return m_column; }
private:
	std::string_view m_text;
	int m_lineNo;
	int m_column;
};
