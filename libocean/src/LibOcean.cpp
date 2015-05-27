#include "LibOcean.hpp"
#include "Stack.hpp"

namespace Ocean
{
	sprawl::collections::HashMap<BoundFunction, sprawl::KeyAccessor<BoundFunction, sprawl::String>> namedNativeFunctions;
}

namespace OceanStatic
{
	static void add(Stack& stack)
	{
		int64_t val1 = stack.Consume().asInt;
		int64_t val2 = stack.Consume().asInt;
		stack.Push(OceanValue(val1 + val2));
	}

	static void mult(Stack& stack)
	{
		int64_t val1 = stack.Consume().asInt;
		int64_t val2 = stack.Consume().asInt;
		stack.Push(OceanValue(val1 * val2));
	}

	static void div(Stack& stack)
	{
		int64_t val1 = stack.Consume().asInt;
		int64_t val2 = stack.Consume().asInt;
		stack.Push(OceanValue(val1 / val2));
	}

	static void sub(Stack& stack)
	{
		int64_t val1 = stack.Consume().asInt;
		int64_t val2 = stack.Consume().asInt;
		stack.Push(OceanValue(val1 - val2));
	}

	static void print(Stack& stack)
	{
		int64_t value = stack.Consume().asInt;
		printf("%ld\n", value);
	}
}

void Ocean::Bind(sprawl::String const& name, BoundFunction::FunctionType function, int nParams, bool isConstExpr /*= false*/)
{
	namedNativeFunctions.insert(BoundFunction(function, nParams, isConstExpr), name);
}

void Ocean::Install()
{
	Bind("add", OceanStatic::add, 2, true);
	Bind("mult", OceanStatic::mult, 2, true);
	Bind("div", OceanStatic::div, 2, true);
	Bind("sub", OceanStatic::sub, 2, true);
	Bind("print", OceanStatic::print, 1);
}
