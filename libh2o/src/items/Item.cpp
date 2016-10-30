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
