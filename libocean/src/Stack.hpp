#pragma once

#include <cstdint>

class Stack
{
public:
	Stack()
		: m_values()
		, m_size(0)
	{
		//NOP
	}

	inline int64_t Consume()
	{
		return m_values[--m_size];
	}

	inline int64_t Read()
	{
		return m_values[m_size-1];
	}

	inline void Pop(int64_t amount)
	{
		m_size -= amount;
	}

	inline void Push(int64_t value)
	{
		m_values[m_size++] = value;
	}

	inline void SwapWithTop(int64_t location)
	{
		location = m_size - location - 1;
		int64_t tmp = m_values[m_size-1];
		m_values[m_size-1] = m_values[location];
		m_values[location] = tmp;
	}

	inline void CopyToTop(int64_t location)
	{
		m_values[m_size] = m_values[m_size - location - 1];
		++m_size;
	}

private:
	int64_t m_values[32768];
	int64_t m_size;
};
