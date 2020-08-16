#include "GroupItem.hpp"
#include "../matches/Match.hpp"
#include "../Logging.hpp"
#include "../../../libocean/src/BytecodeWriter.hpp"
#include "../H2OCompiler.hpp"

/*virtual*/ GroupItem::GroupItem(const std::string_view& name, std::list<std::string_view>&& names)
	: Item(name)
	, m_names(std::move(names))
{

}

Match* GroupItem::Match(H2OCompiler const& compiler, TokenList const& tokens) /*override*/
{
	for(auto& name : m_names)
	{
		Item* item = compiler.GetItem(name);
		::Match* match = item->Match(compiler, tokens);
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
