#include "LibOcean.hpp"
#include "Stack.hpp"
#include "OceanBindings.hpp"

namespace Ocean
{
	sprawl::collections::BasicHashMap<sprawl::String, BoundFunction> namedNativeFunctions;
}

namespace OceanStatic
{
	static int64_t add(int64_t val1, int64_t val2)
	{
		return val1 + val2;
	}

	static int64_t mult(int64_t val1, int64_t val2)
	{
		return val1 * val2;
	}

	static int64_t div(int64_t val1, int64_t val2)
	{
		return val1 / val2;
	}

	static int64_t sub(int64_t val1, int64_t val2)
	{
		return val1 - val2;
	}

	static void print(int64_t value)
	{
		printf("%ld\n", value);
	}
}

void Ocean::Install()
{
	Bind<decltype(OceanStatic::add), OceanStatic::add>("add", true);
	Bind<decltype(OceanStatic::mult), OceanStatic::mult>("mult", true);
	Bind<decltype(OceanStatic::div), OceanStatic::div>("div", true);
	Bind<decltype(OceanStatic::sub), OceanStatic::sub>("sub", true);
	Bind<decltype(OceanStatic::print), OceanStatic::print>("print", false);
}
