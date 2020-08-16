#pragma once

#include <string_view>
#include "../TokenList.hpp"

class Match;
class BytecodeWriter;
class H2OCompiler;

class Item
{
public:
	Item(std::string_view const& name);

	std::string_view const& GetName();

	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) = 0;

	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) = 0;
private:
	std::string_view m_name;
};
