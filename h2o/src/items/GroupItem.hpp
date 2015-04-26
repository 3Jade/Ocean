#pragma once

#include "Item.hpp"
#include <sprawl/collections/List.hpp>

class GroupItem : public Item
{
public:
	GroupItem(sprawl::String const& name, sprawl::collections::List<const char* const>&& names);

	virtual ::Match* Match(TokenList const& tokens) override;
	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) override;

	static sprawl::collections::HashMap<GroupItem, sprawl::KeyAccessor<GroupItem, sprawl::String>> ms_groups;
private:
	sprawl::collections::List<const char* const> m_names;
};

inline GroupItem& AddGroup(sprawl::String const& name, std::initializer_list<const char* const> names)
{
	sprawl::collections::List<const char* const> nameList;

	for(auto name : names)
	{
		nameList.PushBack(name);
	}

	auto it = GroupItem::ms_groups.insert(GroupItem(name, std::move(nameList)), name);
	Item::ms_allItems.insert(&it.Value(), name);
	return it.Value();
}
