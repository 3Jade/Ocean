#pragma once

#include <sprawl/string/String.hpp>
#include <sprawl/collections/HashMap.hpp>
#include "../TokenList.hpp"
#include <sprawl/collections/ForwardList.hpp>

class Match;
class BytecodeWriter;

class Item
{
public:
	Item(sprawl::String const& name);

	sprawl::String const& GetName();

	virtual ::Match* Match(TokenList const& tokens) = 0;

	virtual bool Translate(BytecodeWriter& writer, ::Match const& match) = 0;

	static sprawl::collections::HashMap<Item*, sprawl::KeyAccessor<Item*, sprawl::String>> ms_allItems;
	static sprawl::collections::ForwardList<Item*> ms_topLevelItems;
private:
	sprawl::String m_name;
};

inline void AddTopLevelItem(sprawl::String const& itemName)
{
	Item::ms_topLevelItems.PushFront(Item::ms_allItems.get(itemName));
}
