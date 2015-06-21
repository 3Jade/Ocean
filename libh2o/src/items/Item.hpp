#pragma once

#include <sprawl/string/String.hpp>
#include "../TokenList.hpp"
#include <sprawl/collections/ForwardList.hpp>

class Match;
class BytecodeWriter;
class H2OCompiler;

class Item
{
public:
	Item(sprawl::String const& name);

	sprawl::String const& GetName();

	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) = 0;

	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) = 0;
private:
	sprawl::String m_name;
};
