#include <cstdio>
#include <cstdint>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <sprawl/collections/HashMap.hpp>
#include <sprawl/string/String.hpp>

#include "../../libocean/src/OpCode.hpp"
#include "../../libocean/src/Stack.hpp"
#include "../../libocean/src/LibOcean.hpp"
#include "../../libh2o/src/h2o.hpp"

std::string FormatString(char const* const message, ...) {
	va_list ap;
	
	va_start(ap, message);
	size_t length = vsnprintf(nullptr, 0, message, ap);
	va_end(ap);
	
	char* buffer = reinterpret_cast<char*>(alloca(length+1));
	va_start(ap, message);
	vsnprintf(buffer, length+1, message, ap);
	va_end(ap);
	return std::string(buffer, length);
}

#define READ(type) *reinterpret_cast<type const*>(bytecode)
#define ASSERT(condition, message, ...) do{ if(!(condition)) { throw std::runtime_error(FormatString(message, ## __VA_ARGS__)); } }while(false)

struct Exit
{
	int64_t returnCode;
};

class OceanContext
{
private:
	Stack stack;
	Stack callStack;
	char* bytecodeStart;
	char* bytecode;
	long bytecodeSize;
	OceanValue* variables;
	int64_t varSize;
	
	
	static void PUSH(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->stack.Push(value);
	}
	
	static void POP(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->stack.Pop(value.asInt);
	}

	static void COPY(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->stack.CopyToTop(value.asInt);
	}
	
	static void SWAP(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->stack.SwapWithTop(value.asInt);
	}
	
	static void JUMP(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->bytecode = context->bytecodeStart + value.asInt;
	}
	static void JUMPIF(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		if(context->stack.Consume().asInt != 0)
		{
			context->bytecode = context->bytecodeStart + value.asInt;
		}
	}
	
	static void JUMPNIF(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		if(context->stack.Consume().asInt == 0)
		{
			context->bytecode = context->bytecodeStart + value.asInt;
		}
	}
	
	static void CALL(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->callStack.Push(reinterpret_cast<int64_t>(context->bytecode));
		context->bytecode = reinterpret_cast<char*>(value.asInt);
	}
	
	static void CALLN(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		reinterpret_cast<Ocean::BoundFunction::FunctionType>(value.asInt)(context->stack);
	}
	
	static void RETURN(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->varSize -= value.asInt;
		context->bytecode = reinterpret_cast<char*>(context->callStack.Consume().asInt);
	}
	
	static void DECL(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->varSize += value.asInt;
	}
	
	static void LOAD(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->stack.Push(context->variables[context->varSize + value.asInt]);
	}
	
	static void STORE(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->variables[context->varSize + value.asInt] = context->stack.Consume();
	}
	
	static void MEMO(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->variables[context->varSize + value.asInt] = context->stack.Read();
	}
	
	static void DEL(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		context->varSize -= value.asInt;
	}
	
	static void EXIT(OceanContext* context, OceanValue value)
	{
#if DEBUG_TRACE
#if DEBUG_WITH_CONTEXT_PTRS
		printf("%p ", context);
#endif
		printf(__FUNCTION__);
		printf(" %ld\n", value.asInt);
#endif
		throw Exit{value.asInt};
	}
	
	static void CREATE(OceanContext* context, OceanValue value)
	{}
	static void GET(OceanContext* context, OceanValue value)
	{}
	static void SET(OceanContext* context, OceanValue value)
	{}
	static void DESTROY(OceanContext* context, OceanValue value)
	{}
	
	typedef std::underlying_type<OpCode>::type OpType;
	typedef void(*OpFn)(OceanContext*, OceanValue);
	static OpFn funcs[static_cast<OpType>(OpCode::MAX)];
	
public:
	static void Install()
	{
		funcs[static_cast<OpType>(OpCode::PUSH)]    = &OceanContext::PUSH;
		funcs[static_cast<OpType>(OpCode::POP)]     = &OceanContext::POP;
		funcs[static_cast<OpType>(OpCode::COPY)]    = &OceanContext::COPY;
		funcs[static_cast<OpType>(OpCode::SWAP)]    = &OceanContext::SWAP;
		funcs[static_cast<OpType>(OpCode::JUMP)]    = &OceanContext::JUMP;
		funcs[static_cast<OpType>(OpCode::JUMPIF)]  = &OceanContext::JUMPIF;
		funcs[static_cast<OpType>(OpCode::JUMPNIF)] = &OceanContext::JUMPNIF;
		funcs[static_cast<OpType>(OpCode::CALL)]    = &OceanContext::CALL;
		funcs[static_cast<OpType>(OpCode::CALLN)]   = &OceanContext::CALLN;
		funcs[static_cast<OpType>(OpCode::CALLND)]  = &OceanContext::CALLN;
		funcs[static_cast<OpType>(OpCode::RETURN)]  = &OceanContext::RETURN;
		funcs[static_cast<OpType>(OpCode::DECL)]    = &OceanContext::DECL;
		funcs[static_cast<OpType>(OpCode::LOAD)]    = &OceanContext::LOAD;
		funcs[static_cast<OpType>(OpCode::STORE)]   = &OceanContext::STORE;
		funcs[static_cast<OpType>(OpCode::DEL)]     = &OceanContext::DEL;
		funcs[static_cast<OpType>(OpCode::MEMO)]    = &OceanContext::MEMO;
		funcs[static_cast<OpType>(OpCode::EXIT)]    = &OceanContext::EXIT;
		funcs[static_cast<OpType>(OpCode::CREATE)]  = &OceanContext::CREATE;
		funcs[static_cast<OpType>(OpCode::GET)]     = &OceanContext::GET;
		funcs[static_cast<OpType>(OpCode::SET)]     = &OceanContext::SET;
		funcs[static_cast<OpType>(OpCode::DESTROY)] = &OceanContext::DESTROY;
	}
	
	~OceanContext()
	{
		delete[] variables;
	}

	OceanContext(char* bytecode_, long size)
		: stack()
		, callStack()
		, bytecodeStart(bytecode_)
		, bytecode(bytecode_)
		, bytecodeSize(size)
		, variables(new OceanValue[32768])
		, varSize(0)
	{
		int64_t magic = *reinterpret_cast<int64_t const*>(bytecode);
		ASSERT(magic == 0xEA0C, "Invalid bytecode. (Bad Magic: %ld)", magic);
	
		bytecode += sizeof(int64_t);
		int64_t stringsLength = READ(int64_t);
		bytecode += sizeof(int64_t);
		ASSERT(stringsLength >= 0, "Invalid bytecode. (Negative Length Strings Table)");
		ASSERT(stringsLength + sizeof(int64_t) * 2 < size, "Invalid bytecode. (Invalid Strings Table Length: %ld)", stringsLength);
		char const* stringsStart = bytecode;
		char const* stringsEnd = bytecode + stringsLength;
	
		sprawl::collections::BasicHashMap<int64_t, sprawl::String> strings;
		sprawl::collections::BasicHashMap<int64_t, Ocean::BoundFunction::FunctionType> nativeFunctions;
		sprawl::collections::BasicHashMap<int64_t, Ocean::BoundFunction::FunctionType> nonDestructiveFunctions;
	
		int64_t stringId = 0;
		while(bytecode < stringsEnd)
		{
			int64_t strLength = READ(int64_t);
			bytecode += sizeof(int64_t);
			sprawl::StringLiteral lit(bytecode, strLength);
			strings.insert(stringId, lit);
			if(Ocean::namedNativeFunctions.has(lit))
			{
				auto& func = Ocean::namedNativeFunctions.get(lit);
				nativeFunctions.insert(stringId, func.function);
				if(func.nonDestructiveFunction)
				{
					nonDestructiveFunctions.insert(stringId, func.nonDestructiveFunction);
				}
			}
			bytecode += strLength;
			++stringId;
		}
	
		ASSERT(bytecode == stringsEnd, "Invalid bytecode. (Corrupt Strings Table)");
		
		int64_t padding = 8 - (stringsLength % 8);
		if(padding == 8) {
			padding = 0;
		}
		bytecode += padding;
	
		int64_t functionsLength = READ(int64_t);
		bytecode += sizeof(int64_t);
		ASSERT(functionsLength >= 0, "Invalid bytecode. (Corrupt Functions Table)");
		ASSERT(functionsLength + stringsLength + sizeof(int64_t) * 3 < size, "Invalid bytecode. (Corrupt Functions Table)");
		char* functionsEnd = bytecode + functionsLength;
	
		sprawl::collections::BasicHashMap<int64_t, char*> functions;
		
		while(bytecode < functionsEnd)
		{
			int64_t funcNameOffset = READ(int64_t);
			bytecode += sizeof(int64_t);
			int64_t functionLength = READ(int64_t);
			bytecode += sizeof(int64_t);
			functions.insert(funcNameOffset, bytecode);
	
			char* functionEnd = bytecode + functionLength;
			while(bytecode < functionEnd)
			{
				OpCode code = READ(OpCode);
				OpFn& func = funcs[static_cast<std::underlying_type<OpCode>::type>(code)];
				ASSERT(func != nullptr, "Invalid bytecode operation: %ld", static_cast<uint64_t>(code));
				memcpy(bytecode, &func, sizeof(int64_t));
				bytecode += sizeof(int64_t);
				if(code == OpCode::CALL)
				{
					int64_t value = READ(int64_t);
					if(nativeFunctions.has(value))
					{
						OpFn& func = funcs[static_cast<std::underlying_type<OpCode>::type>(OpCode::CALLN)];
						bytecode -= sizeof(int64_t);
						memcpy(bytecode, &func, sizeof(int64_t));
						bytecode += sizeof(int64_t);
						memcpy(bytecode, &nativeFunctions.get(value), sizeof(int64_t));
					}
					else
					{
						memcpy(bytecode, &functions.get(value), sizeof(int64_t));
					}
				}
				else if(code == OpCode::CALLND)
				{
					int64_t value = READ(int64_t);
					ASSERT(nonDestructiveFunctions.has(value), "CALLND opcode with no valid native function.");
					OpFn& func = funcs[static_cast<std::underlying_type<OpCode>::type>(OpCode::CALLN)];
					bytecode -= sizeof(int64_t);
					memcpy(bytecode, &func, sizeof(int64_t));
					bytecode += sizeof(int64_t);
					memcpy(bytecode, &nonDestructiveFunctions.get(value), sizeof(int64_t));
				}
				bytecode += sizeof(int64_t);
			}
		}
	
		int64_t globalDataSize = READ(int64_t);
		bytecode += sizeof(int64_t);
		int64_t expectedSize = functionsLength + stringsLength + globalDataSize + sizeof(int64_t) * 4 + padding;
		ASSERT(expectedSize == size, "Invalid bytecode. (Corrupt Global Data %ld != %ld)", expectedSize, size);
		ASSERT(globalDataSize % (sizeof(int64_t) * 2) == 0, "Invalid bytecode. (Corrupt Global Data)");
		char* globalStart = bytecode;
		char* globalEnd = globalStart + globalDataSize;
	
		// Fixup
		OpCode code;
		while(bytecode < globalEnd)
		{
			code = READ(OpCode);
			OpFn& func = funcs[static_cast<std::underlying_type<OpCode>::type>(code)];
			ASSERT(func != nullptr, "Invalid bytecode operation: %ld", static_cast<uint64_t>(code));
			memcpy(bytecode, &func, sizeof(int64_t));
			bytecode += sizeof(int64_t);
			if(code == OpCode::CALL)
			{
				int64_t value = READ(int64_t);
				if(nativeFunctions.has(value))
				{
					OpFn& func = funcs[static_cast<std::underlying_type<OpCode>::type>(OpCode::CALLN)];
					bytecode -= sizeof(int64_t);
					memcpy(bytecode, &func, sizeof(int64_t));
					bytecode += sizeof(int64_t);
					memcpy(bytecode, &nativeFunctions.get(value), sizeof(int64_t));
				}
				else
				{
					memcpy(bytecode, &functions.get(value), sizeof(int64_t));
				}
			}
			else if(code == OpCode::CALLND)
			{
				int64_t value = READ(int64_t);
				ASSERT(nonDestructiveFunctions.has(value), "CALLND opcode with no valid native function.");
				OpFn& func = funcs[static_cast<std::underlying_type<OpCode>::type>(OpCode::CALLN)];
				bytecode -= sizeof(int64_t);
				memcpy(bytecode, &func, sizeof(int64_t));
				bytecode += sizeof(int64_t);
				memcpy(bytecode, &nonDestructiveFunctions.get(value), sizeof(int64_t));
			}
			bytecode += sizeof(int64_t);
		}
		ASSERT(code == OpCode::EXIT, "Invalid bytecode. (Incorrect footer)");
		bytecode = globalStart;
	}
	
	int Run()
	{
#if DEBUG_TRACE
		printf("%p beginning execution\n", this);
#endif
		try
		{
			for(;;)
			{
				OpFn func = READ(OpFn);
				bytecode += sizeof(int64_t);
				OceanValue value = READ(OceanValue);
				bytecode += sizeof(OceanValue);
				
				func(this, value);
			}
		}
		catch(Exit const& e) {
			return e.returnCode;
		}
	}
	
};

/*static*/ OceanContext::OpFn OceanContext::funcs[static_cast<std::underlying_type<OpCode>::type>(OpCode::MAX)] = {0};

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		puts("Syntax: ocean <file>");
		return 1;
	}

	Ocean::Install();
	OceanContext::Install();

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

	int ret = OceanContext(fileBuffer, fileSize).Run();
	delete[] fileBuffer;
	return ret;
}
