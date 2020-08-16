#include <stdint.h>
#include <string_view>
#include <list>
#include <SkipProbe/SkipProbe.hpp>
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

	void StartFunction(std::string_view const& name);
	void EndFunction();

	void Stack_Push(int64_t value);
	void Stack_Push(std::string_view const& value);
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

	void Function_Call(std::string_view const& name);
	void Function_Return();

	void Variable_Declare(int64_t stringId);
	void Variable_Load(int64_t stringId);
	void Variable_Store(int64_t stringId);
	void Variable_Memo(int64_t stringId);

	void OceanObj_Create(std::string_view const& className);
	void OceanObj_GetAttr(std::string_view const& attrName);
	void OceanObj_SetAttr(std::string_view const& attrName);
	void OceanObj_Destroy();

	void Exit(int64_t exitCode);

	Instruction const* GetCurrentInstruction();
	int64_t GetStringOffset(std::string_view const& string) const;

	std::string Finish();

private:

	struct ScopeData
	{
		ScopeData() : variables(), data(), varCount(0) {}
		SkipProbe::HashMap<int64_t, int64_t> variables;
		std::list<Instruction> data;
		int64_t varCount;
	};

	struct FunctionData
	{
		FunctionData(int64_t id) : functionId(id), scope() {}
		int64_t functionId;
		ScopeData scope;
	};
	
	void Optimize(ScopeData& scope);
	
#if DEBUG_TRACE
	size_t deletedInstructions{0};
#endif
	
	std::list<FunctionData> m_functions;
	mutable std::list<std::string_view> m_strings;
	ScopeData m_globalData;

	mutable SkipProbe::HashMap<std::string_view, int64_t> m_stringOffsets;

	std::list<Instruction>* m_currentBuilder;
	std::list<std::list<Instruction>*> m_builderStack;

	ScopeData* m_currentScope;
	std::list<ScopeData*> m_scopeStack;
	
	SkipProbe::HashMap<int64_t, int64_t> m_funcParameterCounts;
};

static_assert(sizeof(int64_t) == sizeof(double), "Double is not 64 bits and is required to be.");
