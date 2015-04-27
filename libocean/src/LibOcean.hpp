class Stack;
#include <sprawl/collections/HashMap.hpp>
#include <sprawl/string/String.hpp>

namespace Ocean
{
	struct BoundFunction
	{
		typedef void(*FunctionType)(Stack&);

		BoundFunction(FunctionType function_, int nParams_, bool isConstExpr_)
			: function(function_)
			, nParams(nParams_)
			, isConstExpr(isConstExpr_)
		{
			//NOP
		}

		FunctionType function;
		int nParams;
		bool isConstExpr;
	};

	void Bind(sprawl::String const& name, BoundFunction::FunctionType function, int nParams, bool isConstExpr = false);
	void Install();

	extern sprawl::collections::HashMap<BoundFunction, sprawl::KeyAccessor<BoundFunction, sprawl::String>> namedNativeFunctions;
}
