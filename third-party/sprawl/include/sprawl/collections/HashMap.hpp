#pragma once

//Support for legacy compilers lacking variadic template support
#if (defined(_WIN32) && _MSC_VER < 1800)
/*TODO: Support these too
|| (defined(__clang__) && (__clang_major__ < 2 || (__clang_major__ == 2 && __clang_minor__ < 9))) \
	|| (!defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 3))) \
	|| defined(SPRAWL_NO_VARIADIC_TEMPLATES)*/

#include "hashmap/HashMap_Windows.hpp"

namespace sprawl
{
	namespace collections
	{
		template<typename ValueType>
		class HashSet : public HashMap<ValueType, SelfAccessor<ValueType>, _MAX_NIL_LIST>
		{
			//
		};

		template<typename KeyType, typename ValueType>
		class BasicHashMap : public HashMap<ValueType, KeyAccessor<ValueType, KeyType>, _MAX_NIL_LIST>
		{
			//
		};
	}
}
#else
#include "hashmap/HashMap_Variadic.hpp"

namespace sprawl
{
	namespace collections
	{
		template<typename ValueType>
		using HashSet = HashMap<ValueType, SelfAccessor<ValueType>>;

		template<typename KeyType, typename ValueType>
		using BasicHashMap = HashMap<ValueType, KeyAccessor<ValueType, KeyType>>;

		template<typename KeyType, typename ValueType, KeyType(ValueType::*function)()>
		using MemberHashMap = HashMap<ValueType, MemberAccessor<ValueType, KeyType, function>>;

		template<typename KeyType, typename ValueType, KeyType(ValueType::*function)() const>
		using ConstMemberHashMap = HashMap<ValueType, ConstMemberAccessor<ValueType, KeyType, function>>;

		template<typename KeyType, typename ValueType, KeyType(std::remove_reference<decltype(*(ValueType(nullptr)))>::type::*function)()>
		using PtrMemberHashMap = HashMap<ValueType, PtrMemberAccessor<typename std::remove_reference<decltype(*(ValueType(nullptr)))>::type, KeyType, function, ValueType>>;

		template<typename KeyType, typename ValueType, KeyType(std::remove_reference<decltype(*(ValueType(nullptr)))>::type::*function)() const>
		using PtrConstMemberHashMap = HashMap<ValueType, PtrConstMemberAccessor<typename std::remove_reference<decltype(*(ValueType(nullptr)))>::type, KeyType, function, ValueType>>;

		template<typename KeyType, typename ValueType, KeyType(*function)(ValueType*)>
		using FunctionHashMap = HashMap<ValueType, FunctionAccessor<ValueType, KeyType, function>>;

		template<typename KeyType, typename ValueType, KeyType(*function)(typename std::remove_reference<decltype(*(ValueType(nullptr)))>::type*)>
		using PtrFunctionHashMap = HashMap<ValueType, PtrFunctionAccessor<typename std::remove_reference<decltype(*(ValueType(nullptr)))>::type, KeyType, function, ValueType>>;
	}
}
#endif
