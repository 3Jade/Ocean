#include "BytecodeWriter.hpp"
#include "Stack.hpp"
#include <cassert>
#include <SkipProbe/SkipProbe.hpp>
#include <vector>
#include <unordered_set>

#define APPEND_OP(itemList, opCode, value) (itemList).push_back({opCode, OceanValue(value)})

BytecodeWriter::BytecodeWriter()
	: m_functions()
	, m_strings()
	, m_globalData()
	, m_stringOffsets()
	, m_currentBuilder(&m_globalData.data)
	, m_builderStack()
	, m_currentScope(&m_globalData)
	, m_scopeStack()
{
	m_builderStack.push_back(m_currentBuilder);
	m_scopeStack.push_back(m_currentScope);
}

void BytecodeWriter::StartFunction(std::string_view const& name)
{
	m_functions.push_back(FunctionData(GetStringOffset(name)));
	m_currentBuilder = &m_functions.back().scope.data;
	m_builderStack.push_back(m_currentBuilder);
	m_currentScope = &m_functions.back().scope;
	m_scopeStack.push_back(m_currentScope);
}

void BytecodeWriter::EndFunction()
{
	m_builderStack.pop_back();
	m_currentBuilder = m_builderStack.back();
	m_scopeStack.pop_back();
	m_currentScope = m_scopeStack.back();
}

void BytecodeWriter::Stack_Push(int64_t value)
{
	APPEND_OP(*m_currentBuilder, OpCode::PUSH, value);
}

void BytecodeWriter::Stack_Push(std::string_view const& value)
{
	APPEND_OP(*m_currentBuilder, OpCode::PUSH, GetStringOffset(value));
}

void BytecodeWriter::Stack_Push(double value)
{
	APPEND_OP(*m_currentBuilder, OpCode::PUSH, value);
}

void BytecodeWriter::Stack_Push(OceanValue value)
{
	APPEND_OP(*m_currentBuilder, OpCode::PUSH, value);
}

void BytecodeWriter::Stack_Pop(int64_t numItems)
{
	APPEND_OP(*m_currentBuilder, OpCode::POP, numItems);
}

void BytecodeWriter::Stack_SwapWithTop(int64_t depth)
{
	APPEND_OP(*m_currentBuilder, OpCode::SWAP, depth);
}

void BytecodeWriter::Stack_CopyToTop(int64_t depth)
{
	APPEND_OP(*m_currentBuilder, OpCode::COPY, depth);
}

void BytecodeWriter::Jump(int64_t target)
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMP, target);
}

void BytecodeWriter::JumpIf(int64_t target)
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMPIF, target);
}

void BytecodeWriter::JumpIfNot(int64_t target)
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMPNIF, target);
}

BytecodeWriter::DeferredJump BytecodeWriter::Jump_DeferTarget()
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMP, I64(0));
	return DeferredJump(&m_currentBuilder->back());
}

BytecodeWriter::DeferredJump BytecodeWriter::JumpIf_DeferTarget()
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMPIF, I64(0));
	return DeferredJump(&m_currentBuilder->back());
}

BytecodeWriter::DeferredJump BytecodeWriter::JumpIfNot_DeferTarget()
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMPNIF, I64(0));
	return DeferredJump(&m_currentBuilder->back());
}

void BytecodeWriter::Function_Call(std::string_view const& name)
{
	auto it = Ocean::namedNativeFunctions.find(name);
	if(it.Valid())
	{
		Ocean::BoundFunction& func = it->value;
		if(func.isConstExpr)
		{
			int i = 0;
			bool canBeConst = true;
			auto it2 = m_currentBuilder->rbegin();
			while(i < func.nParams)
			{
				if(it2->op != OpCode::PUSH)
				{
					canBeConst = false;
					break;
				}
				++it2;
				++i;
			}
			if(canBeConst)
			{
				Stack stack;
				for(int i = 0; i < func.nParams; ++i)
				{
					stack.Push(OceanValue(m_currentBuilder->back().value.asInt));
					m_currentBuilder->pop_back();
#if DEBUG_TRACE
					++deletedInstructions;
#endif
				}
				func.function(stack);
				int64_t result = stack.Consume().asInt;
				APPEND_OP(*m_currentBuilder, OpCode::PUSH, result);
				return;
			}
		}
		if(func.nonDestructiveFunction)
		{
			m_funcParameterCounts.Insert(GetStringOffset(name), func.nParams);
			APPEND_OP(*m_currentBuilder, OpCode::CALLND, GetStringOffset(name));
		}
	}
	APPEND_OP(*m_currentBuilder, OpCode::CALL, GetStringOffset(name));
}

void BytecodeWriter::Function_Return()
{
	APPEND_OP(*m_currentBuilder, OpCode::RETURN, I64(0));
}

void BytecodeWriter::Variable_Declare(int64_t stringId)
{
	++m_currentScope->varCount;
	m_currentScope->variables.Insert(stringId, -m_currentScope->varCount);
	APPEND_OP(*m_currentBuilder, OpCode::STORE, -m_currentScope->varCount);
}

void BytecodeWriter::Variable_Load(int64_t stringId)
{
	APPEND_OP(*m_currentBuilder, OpCode::LOAD, m_currentScope->variables.Get(stringId));
}

void BytecodeWriter::Variable_Store(int64_t stringId)
{
	APPEND_OP(*m_currentBuilder, OpCode::STORE, m_currentScope->variables.Get(stringId));
}

void BytecodeWriter::Variable_Memo(int64_t stringId)
{
	APPEND_OP(*m_currentBuilder, OpCode::MEMO, m_currentScope->variables.Get(stringId));
}

void BytecodeWriter::OceanObj_Create(std::string_view const& className)
{
	APPEND_OP(*m_currentBuilder, OpCode::CREATE, GetStringOffset(className));
}

void BytecodeWriter::OceanObj_GetAttr(std::string_view const& attrName)
{
	APPEND_OP(*m_currentBuilder, OpCode::GET, GetStringOffset(attrName));
}

void BytecodeWriter::OceanObj_SetAttr(std::string_view const& attrName)
{
	APPEND_OP(*m_currentBuilder, OpCode::SET, GetStringOffset(attrName));
}

void BytecodeWriter::OceanObj_Destroy()
{
	APPEND_OP(*m_currentBuilder, OpCode::DESTROY, I64(0));
}

void BytecodeWriter::Exit(int64_t exitCode)
{
	APPEND_OP(*m_currentBuilder, OpCode::EXIT, exitCode);
}

BytecodeWriter::Instruction const* BytecodeWriter::GetCurrentInstruction()
{
	return &m_currentBuilder->back();
}

void BytecodeWriter::Optimize(ScopeData& scope)
{
	auto& data = scope.data;
	// Find CALLND/CALL pairs and choose only one of them, delete the other
	// CALLND is an optimization for the case of LOAD N; CALL; LOAD N where the call is capable of completing without changing the stack
	// This selects a version of the call that reads the stack without popping from it and doesn't write to it
	for(auto it = data.begin(); it != data.end(); )
	{
		auto prior_it = it++;
		if(it->op == OpCode::CALLND)
		{
			int64_t nParams = m_funcParameterCounts.Get(it->value.asInt);
			
			auto callnd_it = it++;
			auto call_it = it++;

			std::vector<int64_t> prev_stack;
			
			bool can_optimize = true;
			
			auto backward_it = callnd_it;
			--backward_it;
			bool pop = false;
			for(;backward_it != data.begin(); --backward_it)
			{
				if(backward_it->op == OpCode::CALL)
				{
					can_optimize = false;
					break;
				}
				
				if(backward_it->op == OpCode::POP || backward_it->op == OpCode::STORE)
				{
					pop = true;
					continue;
				}
				
				if(backward_it->op == OpCode::LOAD)
				{
					if(pop) {
						pop = false;
						continue;
					}
					prev_stack.push_back(backward_it->value.asInt);
					if(prev_stack.size() == nParams)
					{
						break;
					}
				}
			}
			
			if(!can_optimize)
			{
				data.erase(callnd_it);
				continue;
			}
			
			std::vector<int64_t> next_stack;
			auto forward_it = call_it;
			++forward_it;
			for(;forward_it != data.end(); --forward_it)
			{
				if(forward_it->op == OpCode::CALL)
				{
					can_optimize = false;
					break;
				}
				
				if(forward_it->op == OpCode::POP || forward_it->op == OpCode::STORE)
				{
					next_stack.pop_back();
					continue;
				}
				
				if(forward_it->op == OpCode::LOAD)
				{
					if(pop) {
						pop = false;
						continue;
					}
					next_stack.push_back(forward_it->value.asInt);
					if(next_stack.size() == nParams)
					{
						break;
					}
				}
			}
			
			if(!can_optimize)
			{
				data.erase(callnd_it);
				continue;
			}
			
			if(prev_stack == next_stack)
			{
				data.erase(call_it);
				auto delete_post_load = it++;
				data.erase(delete_post_load);
#if DEBUG_TRACE
				++deletedInstructions;
#endif
			}
			else
			{
				data.erase(callnd_it);
			}
		}
	}	
	
	// Replace pairs of STORE/LOAD on the same value with MEMO
	for(auto it = data.begin(); it != data.end(); )
	{
		auto delete_it = it++;
		if(delete_it->op == OpCode::STORE && it->op == OpCode::LOAD && delete_it->value == it->value)
		{
			it->op = OpCode::MEMO;
#if DEBUG_TRACE
			++deletedInstructions;
#endif
			data.erase(delete_it);
			++it;
		}
	}
	
	// Find variables that have only STORE or MEMO operations and elide them entirely
	std::unordered_set<int64_t> all;
	std::unordered_set<int64_t> loaded;
	for(auto& item : data)
	{
		if(item.op == OpCode::STORE || item.op == OpCode::MEMO || item.op == OpCode::LOAD)
		{
			all.insert(item.value.asInt);
			if(item.op == OpCode::LOAD)
			{
				loaded.insert(item.value.asInt);
			}
		}
	}
	
	if(all.size() != loaded.size())
	{
		SkipProbe::HashMap<int64_t, int64_t> variableRenumber;
		int64_t num = -1;
		
		for(auto& value : loaded)
		{
			all.erase(value);
			variableRenumber.Insert(value, num--);
		}
		
		for(auto it = data.begin(); it != data.end(); )
		{
			auto delete_it = it++;
			if((delete_it->op == OpCode::STORE || delete_it->op == OpCode::MEMO) && all.count(delete_it->value.asInt) != 0)
			{
				data.erase(delete_it);
#if DEBUG_TRACE
				++deletedInstructions;
#endif
			}
		}
		scope.varCount = loaded.size();
		if(scope.varCount != 0)
		{
			for(auto& item : data)
			{
				if(item.op == OpCode::STORE || item.op == OpCode::MEMO || item.op == OpCode::LOAD)
				{
					item.value = OceanValue(variableRenumber.Get(item.value.asInt));
				}
			}
		}
	}
	
	//TODO: Remove elided variables from strings table.
}

std::string BytecodeWriter::Finish()
{
	assert(m_scopeStack.size() == 1);

	unsigned char header[sizeof(int64_t)] = { 0x0C, 0xEA, 0, 0, 0, 0, 0, 0 };
	
	Optimize(m_globalData);
	if(m_globalData.varCount != 0)
	{
		m_globalData.data.push_front({OpCode::DECL, OceanValue(m_globalData.varCount)});
		m_globalData.data.push_back({OpCode::DEL, OceanValue(m_globalData.varCount)});
	}
	m_globalData.data.push_back({OpCode::EXIT, OceanValue(I64(0))});
	
#if DEBUG_TRACE
	size_t instructions = m_globalData.data.size();
#endif
	
	for(auto& func : m_functions)
	{
		Optimize(func.scope);
		if(func.scope.varCount != 0)
		{
			func.scope.data.push_front({OpCode::DECL, OceanValue(func.scope.varCount)});
		}
		for(auto it = func.scope.data.begin(); it != func.scope.data.end(); ++it)
		{
			if(it->op == OpCode::RETURN)
			{
				it->value = func.scope.varCount;
			}
		}
		func.scope.data.push_back({OpCode::RETURN, OceanValue(func.scope.varCount)});
#if DEBUG_TRACE
		instructions += func.scope.data.size();
#endif
	}

	int64_t sizeOfStrings = 0;
	for(auto& str : m_strings)
	{
		sizeOfStrings += sizeof(int64_t) + str.length();
	}
	int64_t padding = 8 - (sizeOfStrings % 8);
	if(padding == 8) {
		padding = 0;
	}
	int64_t paddedSizeOfStrings = sizeOfStrings + padding;

	int64_t sizeOfFunctions = 0;
	for(auto& func : m_functions)
	{
		sizeOfFunctions += sizeof(int64_t);
		sizeOfFunctions += sizeof(int64_t) + (func.scope.data.size() * sizeof(Instruction));
	}

	int64_t sizeOfGlobalData = m_globalData.data.size() * sizeof(Instruction);

	int size = sizeof(header) + paddedSizeOfStrings + sizeOfFunctions + sizeOfGlobalData + sizeof(int64_t) * 3;
	char* outBuffer = new char[size];
	char* buf = outBuffer;

	memcpy(buf, header, sizeof(header));
	buf += sizeof(header);
	memcpy(buf, &sizeOfStrings, sizeof(sizeOfStrings));
	buf += sizeof(sizeOfStrings);
	for(auto& str : m_strings)
	{
		int64_t len = int64_t(str.length());
		memcpy(buf, &len, sizeof(len));
		buf += sizeof(len);
		memcpy(buf, str.data(), len);
		buf += len;
	}
	char paddingBuf[8] = {0};
	memcpy(buf, paddingBuf, padding);
	buf += padding;

	SkipProbe::HashMap<int64_t, int64_t> offsetsNeedingPatched;
	SkipProbe::HashMap<int64_t, int64_t> offsets;

	memcpy(buf, &sizeOfFunctions, sizeof(sizeOfFunctions));
	buf += sizeof(sizeOfFunctions);
	for(auto& func : m_functions)
	{
		memcpy(buf, &func.functionId, sizeof(int64_t));
		buf += sizeof(int64_t);
		int64_t scopeSize = func.scope.data.size() * (sizeof(OpCode) + sizeof(OceanValue));
		memcpy(buf, &scopeSize, sizeof(int64_t));
		buf += sizeof(int64_t);
		for(auto& item : func.scope.data)
		{
			offsets.Insert(int64_t(&item), buf - outBuffer);
			switch(item.op)
			{
				case OpCode::JUMP:
				case OpCode::JUMPIF:
				case OpCode::JUMPNIF:
					offsetsNeedingPatched.Insert(buf - outBuffer, item.value.asInt);
					break;
				default:
					break;
			}

			memcpy(buf, &item.op, sizeof(OpCode));
			buf += sizeof(OpCode);
			memcpy(buf, &item.value, sizeof(OceanValue));
			buf += sizeof(OceanValue);
		}
	}

	memcpy(buf, &sizeOfGlobalData, sizeof(sizeOfGlobalData));
	buf += sizeof(sizeOfGlobalData);
	for(auto& item : m_globalData.data)
	{
		offsets.Insert(int64_t(&item), buf - outBuffer);
		switch(item.op)
		{
			case OpCode::JUMP:
			case OpCode::JUMPIF:
			case OpCode::JUMPNIF:
				offsetsNeedingPatched.Insert(buf - outBuffer, item.value.asInt);
				break;
			default:
				break;
		}

		memcpy(buf, &item.op, sizeof(OpCode));
		buf += sizeof(OpCode);
		memcpy(buf, &item.value, sizeof(OceanValue));
		buf += sizeof(OceanValue);
	}

	for(auto kvp : offsetsNeedingPatched)
	{
		memcpy(outBuffer + kvp.key, &offsets.Get(kvp.value), sizeof(int64_t));
	}

	std::string str(outBuffer, size);
	delete[] outBuffer;
	
#if DEBUG_TRACE
	printf("Optimizer removed %zu / %zu instructions. Final instruction count: %zu\n", deletedInstructions, instructions + deletedInstructions, instructions);
#endif

	return str;
}

int64_t BytecodeWriter::GetStringOffset(std::string_view const& string) const
{
	auto it = m_stringOffsets.find(string);
	if(it.Valid())
	{
		return it->value;
	}
	else
	{
		int64_t offset = m_strings.size();
		m_strings.push_back(string);
		m_stringOffsets.Insert(string, offset);
		return offset;
	}
}



BytecodeWriter::DeferredJump::DeferredJump(Instruction* item)
	: m_item(item)
{
	// NOP
}

void BytecodeWriter::DeferredJump::SetTarget(Instruction const* target)
{
	m_item->value = OceanValue(int64_t(target));
}
