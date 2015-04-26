#include "GroupItem.hpp"
#include "../matches/Match.hpp"
#include "../Logging.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"

/*virtual*/ GroupItem::GroupItem(const sprawl::String& name, sprawl::collections::List<const char* const>&& names)
	: Item(name)
	, m_names(std::move(names))
{

}

Match* GroupItem::Match(TokenList const& tokens) /*override*/
{
	for(auto& name : m_names)
	{
		auto it = Item::ms_allItems.find(name);
		if(it == Item::ms_allItems.end())
		{
			LogWarning(sprawl::String(sprawl::StringLiteral("Unknown item `{}` in group `{}`")).format(name, GetName()), TokenList(nullptr, 0));
			continue;
		}
		Item* item = it.Value();
		::Match* match = item->Match(tokens);
		if(match)
		{
			return match;
		}
	}
	return nullptr;
}

bool GroupItem::Translate(BytecodeWriter& writer, ::Match const& match)
{
	return match.Translate(writer);
}

/*static*/ sprawl::collections::HashMap<GroupItem, sprawl::KeyAccessor<GroupItem, sprawl::String>> GroupItem::ms_groups;
