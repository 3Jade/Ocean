#include "BytecodeWriter.hpp"
#include "Stack.hpp"
#include <cassert>

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

void BytecodeWriter::StartFunction(const sprawl::String& name)
{
	m_functions.push_back(FunctionData(GetStringOffset(name)));
	m_currentBuilder = &m_functions.back().scope.data;
	m_builderStack.push_back(m_currentBuilder);
	m_currentScope = &m_functions.back().scope;
	m_scopeStack.push_back(m_currentScope);
}

void BytecodeWriter::EndFunction()
{
	APPEND_OP(*m_currentBuilder, OpCode::DEL, m_functions.back().scope.varCount);
	m_builderStack.pop_back();
	m_currentBuilder = m_builderStack.back();
	m_scopeStack.pop_back();
	m_currentScope = m_scopeStack.back();
}

void BytecodeWriter::Stack_Push(int64_t value)
{
	APPEND_OP(*m_currentBuilder, OpCode::PUSH, value);
}

void BytecodeWriter::Stack_Push(const sprawl::String& value)
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
	APPEND_OP(*m_currentBuilder, OpCode::JUMP, 0L);
	return DeferredJump(&m_currentBuilder->back());
}

BytecodeWriter::DeferredJump BytecodeWriter::JumpIf_DeferTarget()
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMPIF, 0L);
	return DeferredJump(&m_currentBuilder->back());
}

BytecodeWriter::DeferredJump BytecodeWriter::JumpIfNot_DeferTarget()
{
	APPEND_OP(*m_currentBuilder, OpCode::JUMPNIF, 0L);
	return DeferredJump(&m_currentBuilder->back());
}

void BytecodeWriter::Function_Call(const sprawl::String& name)
{
	auto it = Ocean::namedNativeFunctions.find(name);
	if(it.Valid())
	{
		Ocean::BoundFunction& func = *it;
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
				}
				func.function(stack);
				int64_t result = stack.Consume().asInt;
				APPEND_OP(*m_currentBuilder, OpCode::PUSH, result);
				return;
			}
		}
	}
	APPEND_OP(*m_currentBuilder, OpCode::CALL, GetStringOffset(name));
}

void BytecodeWriter::Function_Return()
{
	APPEND_OP(*m_currentBuilder, OpCode::RETURN, 0L);
}

void BytecodeWriter::Variable_Declare(int64_t stringId)
{
	++m_currentScope->varCount;
	m_currentScope->variables.insert(-m_currentScope->varCount, stringId);
	APPEND_OP(*m_currentBuilder, OpCode::STORE, -m_currentScope->varCount);
}

void BytecodeWriter::Variable_Load(int64_t stringId)
{
	APPEND_OP(*m_currentBuilder, OpCode::LOAD, m_currentScope->variables.get(stringId));
}

void BytecodeWriter::Variable_Store(int64_t stringId)
{
	APPEND_OP(*m_currentBuilder, OpCode::STORE, m_currentScope->variables.get(stringId));
}

void BytecodeWriter::OceanObj_Create(const sprawl::String& className)
{
	APPEND_OP(*m_currentBuilder, OpCode::CREATE, GetStringOffset(className));
}

void BytecodeWriter::OceanObj_GetAttr(const sprawl::String& attrName)
{
	APPEND_OP(*m_currentBuilder, OpCode::GET, GetStringOffset(attrName));
}

void BytecodeWriter::OceanObj_SetAttr(const sprawl::String& attrName)
{
	APPEND_OP(*m_currentBuilder, OpCode::SET, GetStringOffset(attrName));
}

void BytecodeWriter::OceanObj_Destroy()
{
	APPEND_OP(*m_currentBuilder, OpCode::DESTROY, 0L);
}

void BytecodeWriter::Exit(int64_t exitCode)
{
	APPEND_OP(*m_currentBuilder, OpCode::EXIT, exitCode);
}

BytecodeWriter::Instruction const* BytecodeWriter::GetCurrentInstruction()
{
	return &m_currentBuilder->back();
}

sprawl::String BytecodeWriter::Finish()
{
	assert(m_scopeStack.size() == 1);

	unsigned char header[sizeof(int64_t)] = { 0x0C, 0xEA, 0, 0, 0, 0, 0, 0 };

	m_globalData.data.push_front({OpCode::DECL, OceanValue(m_globalData.varCount)});
	m_globalData.data.push_back({OpCode::DEL, OceanValue(m_globalData.varCount)});
	m_globalData.data.push_back({OpCode::EXIT, OceanValue(0L)});

	for(auto& func : m_functions)
	{
		func.scope.data.push_front({OpCode::DECL, OceanValue(func.scope.varCount)});
		for(auto it = func.scope.data.begin(); it != func.scope.data.end(); ++it)
		{
			if(it->op == OpCode::RETURN)
			{
				func.scope.data.insert(it, {OpCode::DEL, OceanValue(func.scope.varCount)});
			}
		}
		func.scope.data.push_back({OpCode::DEL, OceanValue(func.scope.varCount)});
		func.scope.data.push_back({OpCode::RETURN, OceanValue(0L)});
	}

	int64_t sizeOfStrings = 0;
	for(auto& str : m_strings)
	{
		sizeOfStrings += sizeof(int64_t) + str.length();
	}

	int64_t sizeOfFunctions = 0;
	for(auto& func : m_functions)
	{
		sizeOfFunctions += sizeof(int64_t) + (func.scope.data.size() * sizeof(Instruction));
	}

	int64_t sizeOfGlobalData = m_globalData.data.size() * sizeof(Instruction);

	int size = sizeof(header) + sizeOfStrings + sizeOfFunctions + sizeOfGlobalData + sizeof(int64_t) * 3;
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
		memcpy(buf, str.c_str(), len);
		buf += len;
	}

	sprawl::collections::HashMap<int64_t, sprawl::KeyAccessor<int64_t, int64_t>> offsetsNeedingPatched;
	sprawl::collections::HashMap<int64_t, sprawl::KeyAccessor<int64_t, int64_t>> offsets;

	memcpy(buf, &sizeOfFunctions, sizeof(sizeOfFunctions));
	buf += sizeof(sizeOfFunctions);
	for(auto& func : m_functions)
	{
		memcpy(buf, &func.functionId, sizeof(int64_t));
		buf += sizeof(int64_t);
		for(auto& item : func.scope.data)
		{
			offsets.insert(buf - outBuffer, int64_t(&item));
			switch(item.op)
			{
				case OpCode::JUMP:
				case OpCode::JUMPIF:
				case OpCode::JUMPNIF:
					offsetsNeedingPatched.insert(item.value.asInt, buf - outBuffer);
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
		offsets.insert(buf - outBuffer, int64_t(&item));
		switch(item.op)
		{
			case OpCode::JUMP:
			case OpCode::JUMPIF:
			case OpCode::JUMPNIF:
				offsetsNeedingPatched.insert(item.value.asInt, buf - outBuffer);
				break;
			default:
				break;
		}

		memcpy(buf, &item.op, sizeof(OpCode));
		buf += sizeof(OpCode);
		memcpy(buf, &item.value, sizeof(OceanValue));
		buf += sizeof(OceanValue);
	}

	for(auto it = offsetsNeedingPatched.begin(); it.Valid(); ++it)
	{
		memcpy(outBuffer + it.Key(), &offsets.get(it.Value()), sizeof(int64_t));
	}

	sprawl::String str(outBuffer, size);
	delete[] outBuffer;

	return str;
}

int64_t BytecodeWriter::GetStringOffset(sprawl::String const& string) const
{
	auto it = m_stringOffsets.find(string);
	if(it.Valid())
	{
		return it.Value();
	}
	else
	{
		int64_t offset = m_strings.size();
		m_strings.push_back(string);
		m_stringOffsets.insert(offset, string);
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
