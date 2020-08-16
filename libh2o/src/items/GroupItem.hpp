#pragma once

#include "Item.hpp"
#include <list>

class GroupItem : public Item
{
public:
	GroupItem(std::string_view const& name, std::list<std::string_view>&& names);

	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;
private:
	std::list<std::string_view> m_names;
};
