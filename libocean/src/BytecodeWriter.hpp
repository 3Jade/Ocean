#include <stdint.h>
#include <sprawl/string/String.hpp>
#include <list>
#include <sprawl/collections/HashMap.hpp>
#include "OpCode.hpp"
#include "LibOcean.hpp"

class BytecodeWriter
{
public:

	struct Instruction
	{
		OpCode op;
		OceanValue value;
	};

	class DeferredJump
	{
	public:
		DeferredJump(Instruction* builder);
		void SetTarget(Instruction const* target);
	private:
		Instruction* m_item;
	};

	BytecodeWriter();

	void StartFunction(sprawl::String const& name);
	void EndFunction();

	void Stack_Push(int64_t value);
	void Stack_Push(sprawl::String const& value);
	void Stack_Push(double value);
	void Stack_Push(OceanValue value);

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

	void Variable_Declare(int64_t stringId);
	void Variable_Load(int64_t stringId);
	void Variable_Store(int64_t stringId);

	void OceanObj_Create(sprawl::String const& className);
	void OceanObj_GetAttr(sprawl::String const& attrName);
	void OceanObj_SetAttr(sprawl::String const& attrName);
	void OceanObj_Destroy();

	void Exit(int64_t exitCode);

	Instruction const* GetCurrentInstruction();
	int64_t GetStringOffset(sprawl::String const& string) const;

	sprawl::String Finish();

private:


	struct ScopeData
	{
		ScopeData() : variables(), data(), varCount(0) {}
		sprawl::collections::BasicHashMap<int64_t, int64_t> variables;
		std::list<Instruction> data;
		int64_t varCount;
	};

	struct FunctionData
	{
		FunctionData(int64_t id) : functionId(id), scope() {}
		int64_t functionId;
		ScopeData scope;
	};

	std::list<FunctionData> m_functions;
	mutable std::list<sprawl::String> m_strings;
	ScopeData m_globalData;

	mutable sprawl::collections::BasicHashMap<sprawl::String, int64_t> m_stringOffsets;

	std::list<Instruction>* m_currentBuilder;
	std::list<std::list<Instruction>*> m_builderStack;

	ScopeData* m_currentScope;
	std::list<ScopeData*> m_scopeStack;
};

static_assert(sizeof(int64_t) == sizeof(double), "Double is not 64 bits and is required to be.");
