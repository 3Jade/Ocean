#pragma once

class Stack;
#include <sprawl/collections/HashMap.hpp>
#include <sprawl/string/String.hpp>

#ifndef DEBUG_TRACE
#	define DEBUG_TRACE 0
#endif

#ifndef DEBUG_WITH_CONTEXT_PTRS
#	define DEBUG_WITH_CONTEXT_PTRS 0
#endif

namespace Ocean
{
	struct BoundFunction
	{
		typedef void(*FunctionType)(Stack&);

		BoundFunction(FunctionType function_, int nParams_, bool isConstExpr_, FunctionType nonDestructiveFunction_)
			: function(function_)
			, nonDestructiveFunction(nonDestructiveFunction_)
			, nParams(nParams_)
			, isConstExpr(isConstExpr_)
		{
			//NOP
		}

		FunctionType function;
		FunctionType nonDestructiveFunction;
		int nParams;
		bool isConstExpr;
	};
	
	void Install();

	extern sprawl::collections::BasicHashMap<sprawl::String, BoundFunction> namedNativeFunctions;
}

union OceanValue
{
	OceanValue() : asInt(0) {}
	OceanValue(int64_t value) : asInt(value) {}
	OceanValue(double value) : asDouble(value) {}
	OceanValue(bool value) : asBool(value) {}
	OceanValue(void* value) : asObject(value) {}

	int64_t asInt;
	double asDouble;
	bool asBool;
	void* asObject;
};

inline bool operator==(OceanValue const& lhs, OceanValue const& rhs)
{
	return lhs.asInt == rhs.asInt;
}
