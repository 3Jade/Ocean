#pragma once

#include "../../common/specialized.hpp"
#include "../../common/compat.hpp"

namespace sprawl
{
	template<typename A, typename B>
	class MapIterator;

	namespace collections
	{
		template< typename A, typename... B >
		class HashMap;

		namespace detail
		{
			template< typename A, typename B, size_t C, typename... D >
			class HashMap_Impl;

			template<typename ValueType, typename MostInheritedType, size_t index, typename... AdditionalAccessors>
			class AccessorGroup_Impl;

			template<typename ValueType, typename MostInheritedType, size_t index>
			class AccessorGroup_Impl<ValueType, MostInheritedType, index>
			{
			public:
				ValueType& Value()
				{
					return m_value;
				}

				ValueType const& Value() const
				{
					return m_value;
				}

				ValueType& operator*()
				{
					return m_value;
				}

				ValueType* operator->()
				{
					return &m_value;
				}


				ValueType const& operator*() const
				{
					return m_value;
				}

				ValueType const* operator->() const
				{
					return &m_value;
				}

				inline NullAccessor& Accessor(Specialized<index>)
				{
					static NullAccessor accessor;
					return accessor;
				}

				inline NullAccessor const& Accessor(Specialized<index>) const
				{
					static NullAccessor accessor;
					return accessor;
				}

			protected:
				template< typename A, typename... B >
				friend class sprawl::collections::HashMap;
				template< typename A, typename B, size_t C, typename... D >
				friend class sprawl::collections::detail::HashMap_Impl;
				template<typename A, typename B>
				friend class sprawl::MapIterator;

				AccessorGroup_Impl(ValueType const& value)
					: next(nullptr)
					, prev(nullptr)
					, m_value(value)
				{
					//
				}

				AccessorGroup_Impl(AccessorGroup_Impl const& other)
					: next(nullptr)
					, prev(nullptr)
					, m_value(other.m_value)
				{
					//
				}

				inline MostInheritedType* Next(Specialized<index>)
				{
					return nullptr;
				}

				inline MostInheritedType* Prev(Specialized<index>)
				{
					return nullptr;
				}

				inline void SetNext(Specialized<index>, MostInheritedType*)
				{
					//
				}

				inline void SetPrev(Specialized<index>, MostInheritedType*)
				{
					//
				}

				inline size_t Idx(Specialized<index>)
				{
					return -1;
				}

				inline void SetIndex(Specialized<index>, size_t)
				{
					//
				}

				inline size_t GetHash(Specialized<index>)
				{
					return -1;
				}

				inline void SetHash(Specialized<index>, size_t)
				{
					//
				}

				MostInheritedType* next;
				MostInheritedType* prev;
				ValueType m_value;
			};

			template<typename ValueType, typename MostInheritedType, size_t index, typename AccessorType, typename... AdditionalAccessors>
			class AccessorGroup_Impl<ValueType, MostInheritedType, index, AccessorType, AdditionalAccessors...> : public AccessorGroup_Impl<ValueType, MostInheritedType, index+1, AdditionalAccessors...>
			{
			public:
				typedef AccessorGroup_Impl<ValueType, MostInheritedType, index+1, AdditionalAccessors...> Base;

				using Base::Accessor;
				inline AccessorType& Accessor(Specialized<index>)
				{
					return m_thisAccessor;
				}

				inline AccessorType const& Accessor(Specialized<index>) const
				{
					return m_thisAccessor;
				}

			protected:
				template< typename A, typename... B >
				friend class sprawl::collections::HashMap;
				template< typename A, typename B, size_t C, typename... D >
				friend class sprawl::collections::detail::HashMap_Impl;
				template<typename A, typename B>
				friend class sprawl::MapIterator;

				AccessorGroup_Impl(ValueType const& value)
					: Base(value)
					, m_thisAccessor(this->m_value)
					, m_nextThisAccessor(nullptr)
					, m_prevThisAccessor(nullptr)
					, m_thisIdx(0)
				{
					//
				}

				AccessorGroup_Impl(AccessorGroup_Impl const& other)
					: Base(other)
					, m_thisAccessor(other.m_thisAccessor)
					, m_nextThisAccessor(nullptr)
					, m_prevThisAccessor(nullptr)
					, m_thisIdx(0)
				{
					//
				}

				template<typename Param1, typename... Params>
				AccessorGroup_Impl(ValueType const& value, Param1 key, typename std::enable_if<std::is_constructible<typename AccessorType::arg_type const&, Param1>::value>::type* = nullptr)
					: Base(value)
					, m_thisAccessor(this->m_value, key)
					, m_nextThisAccessor(nullptr)
					, m_prevThisAccessor(nullptr)
					, m_thisIdx(0)
				{
					//
				}

				template<typename Param1, typename... Params>
				AccessorGroup_Impl(ValueType const& value, Param1 key, Params... moreKeys, typename std::enable_if<std::is_constructible<typename AccessorType::arg_type const&, Param1>::value>::type* = nullptr)
					: Base(value, moreKeys...)
					, m_thisAccessor(this->m_value, key)
					, m_nextThisAccessor(nullptr)
					, m_prevThisAccessor(nullptr)
					, m_thisIdx(0)
				{
					//
				}

				template<typename Param1>
				AccessorGroup_Impl(ValueType const& value, Param1 firstParam, typename std::enable_if<!std::is_constructible<typename AccessorType::arg_type const&, Param1>::value>::type* = nullptr)
					: Base(value, firstParam)
					, m_thisAccessor(this->m_value)
					, m_nextThisAccessor(nullptr)
					, m_prevThisAccessor(nullptr)
					, m_thisIdx(0)
				{
					//
				}

				template<typename Param1, typename... Params>
				AccessorGroup_Impl(ValueType const& value, Param1 firstParam, Params... moreKeys, typename std::enable_if<!std::is_constructible<typename AccessorType::arg_type const&, Param1>::value>::type* = nullptr)
					: Base(value, firstParam, moreKeys...)
					, m_thisAccessor(this->m_value)
					, m_nextThisAccessor(nullptr)
					, m_prevThisAccessor(nullptr)
					, m_thisIdx(0)
				{
					//
				}

				using Base::Next;
				inline MostInheritedType* Next(Specialized<index>)
				{
					return m_nextThisAccessor;
				}

				using Base::Prev;
				inline MostInheritedType* Prev(Specialized<index>)
				{
					return m_prevThisAccessor;
				}

				using Base::SetNext;
				inline void SetNext(Specialized<index>, MostInheritedType* next)
				{
					m_nextThisAccessor = next;
				}

				using Base::SetPrev;
				inline void SetPrev(Specialized<index>, MostInheritedType* prev)
				{
					m_prevThisAccessor = prev;
				}

				using Base::Idx;
				inline size_t Idx(Specialized<index>)
				{
					return m_thisIdx;
				}

				using Base::SetIndex;
				inline void SetIndex(Specialized<index>, size_t idx)
				{
					m_thisIdx = idx;
				}

				using Base::GetHash;
				inline size_t GetHash(Specialized<index>)
				{
					return m_thisHash;
				}

				using Base::SetHash;
				inline void SetHash(Specialized<index>, size_t hash)
				{
					m_thisHash = hash;
				}

				AccessorType m_thisAccessor;

				MostInheritedType* m_nextThisAccessor;
				MostInheritedType* m_prevThisAccessor;

				size_t m_thisIdx;
				size_t m_thisHash;
			};

			template<typename ValueType, typename... Accessors>
			class AccessorGroup : public AccessorGroup_Impl<ValueType, AccessorGroup<ValueType, Accessors...>, 0, Accessors...>
			{
			public:
				typedef AccessorGroup_Impl<ValueType, AccessorGroup<ValueType, Accessors...>, 0, Accessors...> Base;
				typedef Base* BasePtr;

				AccessorGroup(ValueType const& value)
					: Base(value)
				{
					//
				}

				template<typename... Params>
				AccessorGroup(ValueType const& value, Params... keys)
					: Base(value, keys...)
				{
					//
				}

				template<int i>
				auto Key() const -> decltype(BasePtr(nullptr)->Accessor(Specialized<i>()).Key())
				{
					return this->Accessor(Specialized<i>()).Key();
				}

				auto Key() const -> decltype(BasePtr(nullptr)->Accessor(Specialized<0>()).Key())
				{
					return this->Accessor(Specialized<0>()).Key();
				}
			};
		}
	}
}
