#include "BytecodeWriter.hpp"

#define APPEND_ENTRY(serializer, entry) serializer % entry.asValue

BytecodeWriter::BytecodeWriter()
	: m_functions(true)
	, m_strings(true)
	, m_globalData(true)
	, m_functionNames()
	, m_tempBuilders()
	, m_stringOffsets()
	, m_patchLocations()
	, m_currentBuilder(&m_globalData)
{
	// NOP
}

void BytecodeWriter::StartFunction(const sprawl::String& name)
{
	m_tempBuilders.PushFront(new sprawl::serialization::BinarySerializer(true));
	m_currentBuilder = m_tempBuilders.front();
	m_functionNames.PushFront(name);
}

void BytecodeWriter::EndFunction()
{
	sprawl::String data = m_tempBuilders.front()->Str();

	APPEND_ENTRY(m_functions, BytecodeEntry(GetStringOffset(m_functionNames.front())));

	int32_t padding;
	m_functions % padding;
	m_functions % data;

	m_functionNames.PopFront();
	delete m_tempBuilders.front();
	m_tempBuilders.PopFront();

	if(m_tempBuilders.Empty())
	{
		m_currentBuilder = &m_globalData;
	}
	else
	{
		m_currentBuilder = m_tempBuilders.front();
	}
}

void BytecodeWriter::Stack_Push(int64_t value)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::PUSH));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(value));
}

void BytecodeWriter::Stack_Push(const sprawl::String& value)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::PUSH));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(GetStringOffset(value)));
}

void BytecodeWriter::Stack_Push(double value)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::PUSH));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(value));
}

void BytecodeWriter::Stack_Pop(int64_t numItems)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::POP));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(numItems));
}

void BytecodeWriter::Stack_SwapWithTop(int64_t depth)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::SWAP));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(depth));
}

void BytecodeWriter::Stack_CopyToTop(int64_t depth)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::COPY));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(depth));
}

void BytecodeWriter::Jump(int64_t target)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::JUMP));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(target));
}

void BytecodeWriter::JumpIf(int64_t target)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::JUMPIF));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(target));
}

void BytecodeWriter::JumpIfNot(int64_t target)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::JUMPNIF));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(target));
}

BytecodeWriter::DeferredJump BytecodeWriter::Jump_DeferTarget()
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::JUMP));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(0L));
	return DeferredJump(m_currentBuilder->Size() - sizeof(int64_t), m_currentBuilder, *this);
}

BytecodeWriter::DeferredJump BytecodeWriter::JumpIf_DeferTarget()
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::JUMPIF));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(0L));
	return DeferredJump(m_currentBuilder->Size() - sizeof(int64_t), m_currentBuilder, *this);
}

BytecodeWriter::DeferredJump BytecodeWriter::JumpIfNot_DeferTarget()
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::JUMPNIF));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(0L));
	return DeferredJump(m_currentBuilder->Size() - sizeof(int64_t), m_currentBuilder, *this);
}

void BytecodeWriter::Function_Call(const sprawl::String& name)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::CALL));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(GetStringOffset(name)));
}

void BytecodeWriter::Function_Return()
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::RETURN));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(0L));
}

void BytecodeWriter::Variable_Declare(int64_t variableId)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::DECL));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(variableId));
}

void BytecodeWriter::Variable_Load(int64_t variableId)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::LOAD));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(variableId));
}

void BytecodeWriter::Variable_Store(int64_t variableId)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::STORE));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(variableId));
}

void BytecodeWriter::Variable_Delete(int64_t variableId)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::DEL));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(variableId));
}

void BytecodeWriter::OceanObj_Create(const sprawl::String& className)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::CREATE));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(GetStringOffset(className)));
}

void BytecodeWriter::OceanObj_GetAttr(const sprawl::String& attrName)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::GET));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(GetStringOffset(attrName)));
}

void BytecodeWriter::OceanObj_SetAttr(const sprawl::String& attrName)
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::SET));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(GetStringOffset(attrName)));
}

void BytecodeWriter::OceanObj_Destroy()
{
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(OpCode::DESTROY));
	APPEND_ENTRY((*m_currentBuilder), BytecodeEntry(0L));
}

int64_t BytecodeWriter::GetCurrentOffset()
{
	return m_currentBuilder->Size();
}

sprawl::String BytecodeWriter::Finish()
{
	int64_t headerAsInt64 = 0;
	unsigned char header[sizeof(int64_t)] = { 0x0C, 0xEA, 0, 0, 0, 0, 0, 0 };
	memcpy(&headerAsInt64, header, sizeof(int64_t));
	//int64_t headerAsInt64 = reinterpret_cast<int64_t>(header);

	sprawl::String strings = m_strings.Str();
	sprawl::String functions = m_functions.Str();
	sprawl::String globalData = m_globalData.Str();

	for(auto& patch : m_patchLocations)
	{
		if(patch.builder == &m_functions)
		{
			memcpy(const_cast<char*>(functions.c_str() + patch.offset), &patch.patchValue, sizeof(int64_t));
		}
		else if(patch.builder == &m_globalData)
		{
			memcpy(const_cast<char*>(globalData.c_str() + patch.offset), &patch.patchValue, sizeof(int64_t));
		}
		else if(patch.builder == &m_strings)
		{
			memcpy(const_cast<char*>(strings.c_str() + patch.offset), &patch.patchValue, sizeof(int64_t));
		}
	}

	sprawl::serialization::BinarySerializer final(true);
	APPEND_ENTRY(final, BytecodeEntry(headerAsInt64));
	int32_t padding = 0;
	final % padding;
	final % strings;
	final % padding;
	final % functions;
	final % padding;
	final % globalData;

	return final.Str();
}

void BytecodeWriter::AddPatchLocation(sprawl::serialization::BinarySerializer* builder, int64_t offset, int64_t patchValue)
{
	m_patchLocations.PushFront({builder, offset, patchValue});
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
		int64_t offset = m_strings.Size();
		int32_t padding;
		m_strings % padding;
		m_strings % string;
		m_stringOffsets.insert(offset, string);
		return offset;
	}
}



BytecodeWriter::DeferredJump::DeferredJump(int64_t offset, sprawl::serialization::BinarySerializer* builder, BytecodeWriter& writer)
	: m_offset(offset)
	, m_builder(builder)
	, m_writer(writer)
{
	// NOP
}

void BytecodeWriter::DeferredJump::SetTarget(int64_t target)
{
	m_writer.AddPatchLocation(m_builder, m_offset, target);
}
