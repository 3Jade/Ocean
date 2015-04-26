#pragma once

#include "../items/Item.hpp"

class BytecodeWriter;

class Match
{
public:
	Match(Item& baseItem, size_t endIndex);
	size_t End() const;
	Item& GetBaseItem() const;

	bool Translate(BytecodeWriter& writer) const;

	virtual void Print() = 0;

	virtual void Release() = 0;
private:
	Item& m_baseItem;
	size_t m_endIndex;
};
