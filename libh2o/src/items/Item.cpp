#include "Item.hpp"

Item::Item(std::string_view const& name)
: m_name(name)
{
	//NOP
}

std::string_view const& Item::GetName()
{
	return m_name;
}
