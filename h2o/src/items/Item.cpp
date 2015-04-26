#include "Item.hpp"

Item::Item(const sprawl::String& name)
: m_name(name)
{
	//NOP
}

const sprawl::String&Item::GetName()
{
	return m_name;
}

/*static*/ sprawl::collections::HashMap<Item*, sprawl::KeyAccessor<Item*, sprawl::String>> Item::ms_allItems;
/*static*/ sprawl::collections::ForwardList<Item*> Item::ms_topLevelItems;
