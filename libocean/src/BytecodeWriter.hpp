#include <stdint.h>
#include <sprawl/string/String.hpp>
#include <sprawl/serialization/BinarySerializer.hpp>
#include <sprawl/collections/ForwardList.hpp>
#include <sprawl/collections/HashMap.hpp>
#include "OpCode.hpp"

class BytecodeWriter
{
public:
	class DeferredJump
	{
	public:
		DeferredJump(int64_t offset, sprawl::serialization::BinarySerializer* builder, BytecodeWriter& writer);
		void SetTarget(int64_t target);
	private:
		int64_t m_offset;
		sprawl::serialization::BinarySerializer* m_builder;
		BytecodeWriter& m_writer;
	};

	BytecodeWriter();

	void StartFunction(sprawl::String const& name);
	void EndFunction();

	void Stack_Push(int64_t value);
	void Stack_Push(sprawl::String const& value);
	void Stack_Push(double value);

	void Stack_Pop(int64_t numItems);
	void Stack_SwapWithTop(int64_t depth);
	void Stack_CopyToTop(int64_t depth);

	void Jump(int64_t target);
	void JumpIf(int64_t target);
	void JumpIfNot(int64_t target);
	DeferredJump Jump_DeferTarget();
	DeferredJump JumpIf_DeferTarget();
	DeferredJump JumpIfNot_DeferTarget();

	void Function_Call(sprawl::String const& name);
	void Function_Return();

	void Variable_Declare(int64_t variableId);
	void Variable_Load(int64_t variableId);
	void Variable_Store(int64_t variableId);
	void Variable_Delete(int64_t variableId);

	void OceanObj_Create(sprawl::String const& className);
	void OceanObj_GetAttr(sprawl::String const& attrName);
	void OceanObj_SetAttr(sprawl::String const& attrName);
	void OceanObj_Destroy();

	int64_t GetCurrentOffset();
	int64_t GetStringOffset(sprawl::String const& string) const;

	sprawl::String Finish();

protected:
	void AddPatchLocation(sprawl::serialization::BinarySerializer* builder, int64_t offset, int64_t patchValue);

private:

	union BytecodeEntry
	{
		explicit BytecodeEntry(int64_t value) : asValue(value) {}
		explicit BytecodeEntry(OpCode value) : asValue(static_cast<std::underlying_type<OpCode>::type>(value)) {}
		explicit BytecodeEntry(double value) : asDouble(value) {}
		int64_t asValue;
		double asDouble;
		char asByteStr[sizeof(int64_t)];
	};

	sprawl::serialization::BinarySerializer m_functions;
	mutable sprawl::serialization::BinarySerializer m_strings;
	sprawl::serialization::BinarySerializer m_globalData;

	sprawl::collections::ForwardList<sprawl::String> m_functionNames;
	sprawl::collections::ForwardList<sprawl::serialization::BinarySerializer*> m_tempBuilders;
	mutable sprawl::collections::HashMap<int64_t, sprawl::KeyAccessor<int64_t, sprawl::String>> m_stringOffsets;

	struct PatchLocation
	{
		sprawl::serialization::BinarySerializer* builder;
		int64_t offset;
		int64_t patchValue;
	};
	sprawl::collections::ForwardList<PatchLocation> m_patchLocations;

	sprawl::serialization::BinarySerializer* m_currentBuilder;
};

static_assert(sizeof(int64_t) == sizeof(double), "Double is not 64 bits and is required to be.");
