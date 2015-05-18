#pragma once

#include "Item.hpp"
#include <sprawl/collections/List.hpp>

class GroupItem : public Item
{
public:
	GroupItem(sprawl::String const& name, sprawl::collections::List<sprawl::StringLiteral>&& names);

	virtual ::Match* Match(H2OCompiler const& compiler, TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;
private:
	sprawl::collections::List<sprawl::StringLiteral> m_names;
};
