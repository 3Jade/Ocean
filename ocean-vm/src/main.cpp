#include <cstdio>
#include <cstdint>
#include <sys/stat.h>
#include <unistd.h>
#include <sprawl/collections/HashMap.hpp>
#include <sprawl/string/String.hpp>

#include "../../libocean/src/OpCode.hpp"
#include "../../libocean/src/Stack.hpp"
#include "../../libocean/src/LibOcean.hpp"
#include "../../libh2o/src/h2o.hpp"


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

	sprawl::collections::BasicHashMap<int64_t, sprawl::String> strings;
	sprawl::collections::BasicHashMap<int64_t, Ocean::BoundFunction::FunctionType> nativeFunctions;

	int64_t stringId = 0;
	while(bytecode < stringsEnd)
	{
		int64_t strLength = *READ(int64_t);
		bytecode += sizeof(int64_t);
		sprawl::StringLiteral lit(bytecode, strLength);
		strings.insert(stringId, lit);
		if(Ocean::namedNativeFunctions.has(lit))
		{
			nativeFunctions.insert(stringId, Ocean::namedNativeFunctions.get(lit).function);
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

	sprawl::collections::BasicHashMap<int64_t, char*> functions;

	while(bytecode < functionsEnd)
	{
		int64_t funcNameOffset = *READ(int64_t);
		bytecode += sizeof(int64_t);
		int64_t functionLength = *READ(int64_t);
		bytecode += sizeof(int64_t);
		functions.insert(funcNameOffset, bytecode);

		char* functionEnd = bytecode + functionLength;
		while(bytecode < functionEnd)
		{
			OpCode code = *READ(OpCode);
			bytecode += sizeof(int64_t);
			if(code == OpCode::CALL)
			{
				int64_t value = *READ(int64_t);
				if(nativeFunctions.has(value))
				{
					OpCode code = OpCode::CALLN;
					bytecode -= sizeof(int64_t);
					memcpy(bytecode, &code, sizeof(int64_t));
					bytecode += sizeof(int64_t);
					memcpy(bytecode, &nativeFunctions.get(value), sizeof(int64_t));
				}
				else
				{
					memcpy(bytecode, &functions.get(value), sizeof(int64_t));
				}
			}
			bytecode += sizeof(int64_t);
		}
	}

	OceanValue* variables = new OceanValue[32768];
	int64_t varSize = 0;

	int64_t globalDataSize = *READ(int64_t);
	bytecode += sizeof(int64_t);
	ASSERT(functionsLength + stringsLength + globalDataSize + sizeof(int64_t) * 4 == size, "Invalid bytecode. (Corrupt Global Data %ld != %ld)", functionsLength + stringsLength + globalDataSize + sizeof(int64_t) * 3, size);
	ASSERT(globalDataSize % (sizeof(int64_t) * 2) == 0, "Invalid bytecode. (Corrupt Global Data)");
	char* globalStart = bytecode;
	char* globalEnd = globalStart + globalDataSize;

	while(bytecode < globalEnd)
	{
		OpCode code = *READ(OpCode);
		bytecode += sizeof(int64_t);
		if(code == OpCode::CALL)
		{
			int64_t value = *READ(int64_t);
			if(nativeFunctions.has(value))
			{
				OpCode code = OpCode::CALLN;
				bytecode -= sizeof(int64_t);
				memcpy(bytecode, &code, sizeof(int64_t));
				bytecode += sizeof(int64_t);
				memcpy(bytecode, &nativeFunctions.get(value), sizeof(int64_t));
			}
			else
			{
				memcpy(bytecode, &functions.get(value), sizeof(int64_t));
			}
		}
		bytecode += sizeof(int64_t);
	}
	bytecode -= sizeof(int64_t) * 2;
	ASSERT(*READ(OpCode) == OpCode::EXIT, "Invalid bytecode. (Incorrect footer)");

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
				callStack.Push(reinterpret_cast<int64_t>(bytecode));
				bytecode = reinterpret_cast<char*>(value.asInt);
				break;
			case OpCode::CALLN:
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
				//goto loop;
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

	char filenameBuffer[256];
	char* filename;

	int len = strlen(argv[1]);
	if(len < 4 || argv[1][len-4] != '.' || argv[1][len-3] != 'o' || argv[1][len-2] != 'c' || argv[1][len-1] != 'c')
	{
		sprintf(filenameBuffer, "%s.occ", argv[1]);
		struct stat inFileStat;
		struct stat outFileStat;

		int ret = stat(argv[1], &inFileStat);
		if(ret == -1)
		{
			puts("Cannot open file.");
			return 1;
		}
		ret = stat(filenameBuffer, &outFileStat);
		if(ret == -1 || inFileStat.st_mtime > outFileStat.st_mtime)
		{
			if(!H2O::CompileFile(argv[1], "test.h2o"))
			{
				return 1;
			}
		}
		filename = filenameBuffer;
	}
	else
	{
		filename = argv[1];
	}

	FILE* f = fopen(filename, "r");
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
