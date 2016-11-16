#pragma once

#include "Stack.hpp"

namespace Ocean
{
	template<typename T>
	struct IsVoidReturn
	{
		static constexpr bool value = false;
	};
	
	template<typename... Args>
	struct IsVoidReturn<void(Args...)>
	{
		static constexpr bool value = true;
	};
	
	template<typename T>
	struct GetAs;
	
	template<>
	struct GetAs<int64_t>
	{
		static int64_t Consume(Stack& stack)
		{
			return stack.Consume().asInt;
		}
		static int64_t Read(Stack& stack)
		{
			return stack.Read().asInt;
		}
	};
	
	template<size_t n, typename t_FirstArg, typename... Args>
	struct GetNthParam
	{
		typedef typename GetNthParam<n - 1, Args...>::type type;
	};
	
	template<typename t_FirstArg, typename... Args>
	struct GetNthParam<0, t_FirstArg, Args...>
	{
		typedef t_FirstArg type;
	};
	
	template<typename t_FirstArg>
	struct GetNthParam<0, t_FirstArg>
	{
		typedef t_FirstArg type;
	};
	
	template<size_t n, typename t_FunctionType>
	struct GetNthArgType;
	
	template<size_t n, typename t_ReturnType, typename... Args>
	struct GetNthArgType<n, t_ReturnType(Args...)>
	{
		typedef typename GetNthParam<n, Args...>::type type;
	};
	
	
	template<typename t_FunctionType>
	struct GetReturnType;
	
	template<typename t_ReturnType, typename... Args>
	struct GetReturnType<t_ReturnType(Args...)>
	{
		typedef t_ReturnType type;
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, size_t nArgs, size_t curArg, typename t_ReturnType = typename GetReturnType<t_FunctionType>::type>
	struct FunctionCaller
	{
		template<typename... Args>
		static void Call(Stack& stack, Args&&... args)
		{
			typedef typename GetNthArgType<curArg, t_FunctionType>::type CurrentArgumentType;
			CurrentArgumentType arg = GetAs<CurrentArgumentType>::Consume(stack);
			FunctionCaller<t_FunctionType, t_FunctionPtr, nArgs, curArg + 1>::Call(stack, std::forward<Args>(args)..., std::move(arg));
		}
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, size_t nArgs, typename t_ReturnType>
	struct FunctionCaller<t_FunctionType, t_FunctionPtr, nArgs, nArgs, t_ReturnType>
	{
		template<typename... Args>
		static void Call(Stack& stack, Args&&... args)
		{
			t_ReturnType ret = t_FunctionPtr(std::forward<Args>(args)...);
			stack.Push(OceanValue(ret));
		}
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, size_t nArgs>
	struct FunctionCaller<t_FunctionType, t_FunctionPtr, nArgs, nArgs, void>
	{
		template<typename... Args>
		static void Call(Stack& stack, Args&&... args)
		{
			t_FunctionPtr(std::forward<Args>(args)...);
		}
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, typename t_ReturnType>
	struct FunctionCaller<t_FunctionType, t_FunctionPtr, 0, 0, t_ReturnType>
	{
		static void Call(Stack& stack)
		{
			t_ReturnType ret = t_FunctionPtr();
			stack.Push(OceanValue(ret));
		}
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr>
	struct FunctionCaller<t_FunctionType, t_FunctionPtr, 0, 0, void>
	{
		static void Call(Stack& stack)
		{
			t_FunctionPtr();
		}
	};
	
	
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, size_t nArgs, size_t curArg>
	struct NonDestructiveCaller
	{
		template<typename... Args>
		static void Call(Stack& stack, Args&&... args)
		{
			typedef typename GetNthArgType<curArg, t_FunctionType>::type CurrentArgumentType;
			CurrentArgumentType arg = GetAs<CurrentArgumentType>::Read(stack);
			NonDestructiveCaller<t_FunctionType, t_FunctionPtr, nArgs, curArg + 1>::Call(stack, std::forward<Args>(args)..., std::move(arg));
		}
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, size_t nArgs>
	struct NonDestructiveCaller<t_FunctionType, t_FunctionPtr, nArgs, nArgs>
	{
		template<typename... Args>
		static void Call(Stack& stack, Args&&... args)
		{
			t_FunctionPtr(std::forward<Args>(args)...);
		}
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr>
	struct NonDestructiveCaller<t_FunctionType, t_FunctionPtr, 0, 0>
	{
		static void Call(Stack& stack)
		{
			t_FunctionPtr();
		}
	};
	
	template<typename t_FunctionPtr>
	struct NumArgs
	{
		static constexpr size_t value = 0;
	};
	
	template<typename t_Ret, typename... t_Args>
	struct NumArgs<t_Ret(t_Args...)>
	{
		static constexpr size_t value = sizeof...(t_Args);
	};
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, typename std::enable_if<!IsVoidReturn<t_FunctionType>::value>::type* = nullptr>
	void Bind(sprawl::String const& name, bool isConstExpr = false)
	{
		Ocean::namedNativeFunctions.insert(name, Ocean::BoundFunction(FunctionCaller<t_FunctionType, t_FunctionPtr, NumArgs<t_FunctionType>::value, 0>::Call, NumArgs<t_FunctionType>::value, isConstExpr, nullptr));	
	}
	
	template<typename t_FunctionType, t_FunctionType t_FunctionPtr, typename std::enable_if<IsVoidReturn<t_FunctionType>::value>::type* = nullptr>
	void Bind(sprawl::String const& name, bool isConstExpr)
	{
		Ocean::namedNativeFunctions.insert(name, Ocean::BoundFunction(
			FunctionCaller<t_FunctionType, t_FunctionPtr, NumArgs<t_FunctionType>::value, 0>::Call,
			NumArgs<t_FunctionType>::value, 
			isConstExpr,
			NonDestructiveCaller<t_FunctionType, t_FunctionPtr, NumArgs<t_FunctionType>::value, 0>::Call
		));	
	}
}
