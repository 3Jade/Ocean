#include <cstdio>
#include <cstdint>
#include <sprawl/collections/HashMap.hpp>
#include <sprawl/string/String.hpp>
#include <sprawl/time/time.hpp>

#include "../../libocean/src/OpCode.hpp"
#include "../../libocean/src/Stack.hpp"
#include "../../libocean/src/LibOcean.hpp"


int Process(char* bytecode, long size)
{
	Stack stack;
	Stack callStack;

	char* bytecodeStart = bytecode;

#define READ(type) reinterpret_cast<type const*>(bytecode)
#define ASSERT(condition, message, ...) do{ if(!(condition)) { printf(message, ## __VA_ARGS__); puts(""); return 1; } }while(false)
	int64_t magic = *reinterpret_cast<int64_t const*>(bytecode);
	ASSERT(magic == 0xEA0C, "Invalid bytecode. (Bad Magic: %ld)", magic);

	bytecode += sizeof(int64_t);
	int64_t stringsLength = *READ(int64_t);
	bytecode += sizeof(int64_t);
	ASSERT(stringsLength >= 0, "Invalid bytecode. (Negative Length Strings Table)");
	ASSERT(stringsLength + sizeof(int64_t) * 2 < size, "Invalid bytecode. (Invalid Strings Table Length: %ld)", stringsLength);
	char const* stringsStart = bytecode;
	char const* stringsEnd = bytecode + stringsLength;

	sprawl::collections::HashMap<sprawl::String, sprawl::KeyAccessor<sprawl::String, int64_t>> strings;
	sprawl::collections::HashMap<Ocean::BoundFunction::FunctionType, sprawl::KeyAccessor<Ocean::BoundFunction::FunctionType, int64_t>> nativeFunctions;

	int64_t stringId = 0;
	while(bytecode < stringsEnd)
	{
		int64_t strLength = *READ(int64_t);
		bytecode += sizeof(int64_t);
		sprawl::StringLiteral lit(bytecode, strLength);
		strings.insert(lit, stringId);
		if(Ocean::namedNativeFunctions.has(lit))
		{
			nativeFunctions.insert(Ocean::namedNativeFunctions.get(lit).function, stringId);
		}
		bytecode += strLength;
		++stringId;
	}

	ASSERT(bytecode == stringsEnd, "Invalid bytecode. (Corrupt Strings Table)");

	int64_t functionsLength = *READ(int64_t);
	bytecode += sizeof(int64_t);
	ASSERT(functionsLength >= 0, "Invalid bytecode. (Corrupt Functions Table)");
	ASSERT(functionsLength + stringsLength + sizeof(int64_t) * 3 < size, "Invalid bytecode. (Corrupt Functions Table)");
	char* functionsEnd = bytecode + functionsLength;

	sprawl::collections::HashMap<int64_t, sprawl::KeyAccessor<int64_t, int64_t>> functions;

	bytecode += functionsLength;

	OceanValue* variables = new OceanValue[32768];
	int64_t varSize = 0;

	int64_t globalDataSize = *READ(int64_t);
	bytecode += sizeof(int64_t);
	ASSERT(functionsLength + stringsLength + globalDataSize + sizeof(int64_t) * 4 == size, "Invalid bytecode. (Corrupt Global Data %ld != %ld)", functionsLength + stringsLength + globalDataSize + sizeof(int64_t) * 3, size);
	ASSERT(globalDataSize % (sizeof(int64_t) * 2) == 0, "Invalid bytecode. (Corrupt Global Data)");
	char* globalStart = bytecode;
	char* globalEnd = globalStart + globalDataSize;
	int64_t start = sprawl::time::Now(sprawl::time::Resolution::Milliseconds);

	while(bytecode < globalEnd)
	{
		OpCode code = *READ(OpCode);
		bytecode += sizeof(int64_t);
		if(code == OpCode::CALL)
		{
			int64_t value = *READ(int64_t);
			memcpy(bytecode, &nativeFunctions.get(value), sizeof(int64_t));
		}
		bytecode += sizeof(int64_t);
	}
	bytecode -= sizeof(int64_t) * 2;
	ASSERT(*READ(OpCode) == OpCode::EXIT, "Invalid bytecode. (Incorrect footer)");
	int j = 0;
loop:
	++j;
	if(j <= 1000000)
	{
		bytecode = globalStart;
		for(;;)
		{
			OpCode code = *READ(OpCode);
			bytecode += sizeof(int64_t);
			OceanValue value = *READ(OceanValue);
			bytecode += sizeof(OceanValue);

			switch(code)
			{
				case OpCode::PUSH:
					stack.Push(value);
					break;
				case OpCode::POP:
					stack.Pop(value.asInt);
					break;
				case OpCode::COPY:
					stack.CopyToTop(value.asInt);
					break;
				case OpCode::SWAP:
				{
					stack.SwapWithTop(value.asInt);
					break;
				}
				case OpCode::JUMP:
					bytecode = bytecodeStart + value.asInt;
					break;
				case OpCode::JUMPIF:
					if(stack.Consume().asInt != 0)
					{
						bytecode = bytecodeStart + value.asInt;
					}
					break;
				case OpCode::JUMPNIF:
					if(stack.Consume().asInt == 0)
					{
						bytecode = bytecodeStart + value.asInt;
					}
					break;
				case OpCode::CALL:
					reinterpret_cast<Ocean::BoundFunction::FunctionType>(value.asInt)(stack);
					break;
				case OpCode::RETURN:
					bytecode = reinterpret_cast<char*>(callStack.Consume().asInt);
					break;
				case OpCode::DECL:
					varSize += value.asInt;
					break;
				case OpCode::LOAD:
					stack.Push(variables[varSize + value.asInt]);
					break;
				case OpCode::STORE:
					variables[varSize + value.asInt] = stack.Consume();
					break;
				case OpCode::DEL:
					varSize -= value.asInt;
					break;
				case OpCode::EXIT:
					goto loop;
					return value.asInt;
					break;
				case OpCode::CREATE:
				case OpCode::GET:
				case OpCode::SET:
				case OpCode::DESTROY:
				default:
					break;
			}
		}
	}
	int64_t end = sprawl::time::Now(sprawl::time::Resolution::Milliseconds);
	//printf("Elapsed time: %ld ms\n", end - start);
#undef READ
#undef ASSERT
//	printf("Variables:\n");
//	for(int i = 0; i < stringId; ++i)
//	{
//		printf("\t%.*s - %ld\n", int(strings.get(i).length()), strings.get(i).c_str(), variables[i]);
//	}
//	printf("Stack:\n");
//	for(auto i : stack)
//	{
//		printf("\t%ld\n", i);
//	}
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		puts("Syntax: ocean <file>");
		return 1;
	}

	Ocean::Install();

	FILE* f = fopen(argv[1], "r");
	if(!f)
	{
		puts("Cannot open file.");
		return 1;
	}

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* fileBuffer = new char[fileSize];
	fread(fileBuffer, 1, fileSize, f);

	int ret = Process(fileBuffer, fileSize);
	delete[] fileBuffer;
	return ret;
}
