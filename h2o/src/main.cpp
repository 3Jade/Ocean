#include "H2OCompiler.hpp"
#include "h2o.hpp"
#include "../../libocean/src/BytecodeWriter.hpp"
#include "../../libocean/src/LibOcean.hpp"
#include "H2OMetaLanguage.hpp"

/**
 * @TODO: Token ID assignment, matching during Parse() by assigned ID, perhaps embed match into token?
 * @TODO: Re-entrant/multi-match groups to find most exact match
 * @TODO: Significant whitespace option
 */

#include <cstdio>

#include "matches/ExpressionMatch.hpp"
#include "matches/RegexMatch.hpp"
#include "matches/StringMatch.hpp"

bool declare_var(BytecodeWriter& writer, ExpressionMatch const& match)
{
	auto& initializers = match["initializer"][0];
	if(!initializers.empty())
	{
		if(!initializers[0]->Translate(writer))
		{
			return false;
		}
	}
	else
	{
		writer.Stack_Push(0L);
	}
	RegexMatch const* identifierMatch = static_cast<RegexMatch const*>(match["identifier"][0][0]);
	OceanValue value;
	if(!identifierMatch->GetLiteralValue(writer, value))
	{
		return false;
	}
	writer.Variable_Declare(value.asInt);
	return true;
}

bool mult(BytecodeWriter& writer, ExpressionMatch const& match)
{
	if(!match["LHS"][0][0]->Translate(writer))
	{
		return false;
	}
	if(!match["RHS"][0][0]->Translate(writer))
	{
		return false;
	}
	writer.Function_Call("mult");
	return true;
}

bool add(BytecodeWriter& writer, ExpressionMatch const& match)
{
	if(!match["LHS"][0][0]->Translate(writer))
	{
		return false;
	}
	if(!match["RHS"][0][0]->Translate(writer))
	{
		return false;
	}
	writer.Function_Call("add");
	return true;
}

bool ReadDecInt(BytecodeWriter const& /*writer*/, RegexMatch const& match, OceanValue& outValue)
{
	outValue.asInt = atoi(match.Group(0).GetPtr());
	return true;
}

bool ReadDecFloat(BytecodeWriter const& /*writer*/, RegexMatch const& match, OceanValue& outValue)
{
	outValue.asDouble = atof(match.Group(0).GetPtr());
	return true;
}

bool ReadString(BytecodeWriter const& writer, StringMatch const& match, OceanValue& outValue)
{
	outValue.asInt = writer.GetStringOffset(match.GetString());
	return true;
}

#include <sprawl/time/time.hpp>

int main()
{
	Ocean::Install();

	H2OCompiler compiler;
	compiler.AddExpressionTranslator("mult", mult);
	compiler.AddExpressionTranslator("add", add);
	compiler.AddExpressionTranslator("declare_var", declare_var);

	compiler.AddSpanTranslator("ReadString", ReadString);

	compiler.AddLiteralTranslator("ReadDecInt", ReadDecInt);
	compiler.AddLiteralTranslator("ReadDecFloat", ReadDecFloat);

	FILE* f = fopen("../../../../../test.h2o", "r");

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* fileBuffer = new char[fileSize];
	fread(fileBuffer, 1, fileSize, f);

	g_currentSource = fileBuffer;
	g_currentFile = "test.h2o";

	ProcessH2OFile(fileBuffer, fileSize, compiler);

	char const* const text = "var i = ( 5 + 10 ) * (10+15*6)";

	g_currentSource = text;
	g_currentFile = "<string>";

//	int64_t now = sprawl::time::SteadyNow(sprawl::time::Resolution::Milliseconds);
//	for(int i = 0; i < 10000; ++i)
//	{
//		Compile(sprawl::StringLiteral(text, strlen(text)));
//	}
//	int64_t elapsed = sprawl::time::SteadyNow(sprawl::time::Resolution::Milliseconds) - now;
//	printf("Elapsed: %ld\n", elapsed);

	TokenList tokens;
	if(compiler.Tokenize(sprawl::StringLiteral(text, strlen(text)), tokens))
	{
		sprawl::String result;
		if(compiler.Parse(tokens, result))
		{
			FILE* f = fopen("out2.occ", "w");
			fwrite(result.c_str(), sizeof(char), result.length(), f);
			fclose(f);
		}
	}

	return 0;
}
