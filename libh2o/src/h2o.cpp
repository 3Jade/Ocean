#include "H2OCompiler.hpp"
#include "h2o.hpp"
#include "../../libocean/src/BytecodeWriter.hpp"
#include "../../libocean/src/LibOcean.hpp"
#include "H2OMetaLanguage.hpp"

/**
 * @TODO: Token ID assignment, matching during Parse() by assigned ID, perhaps embed match into token?
 * @TODO: Re-entrant/multi-match groups to find most exact match
 */

#include <cstdio>

#include "matches/ExpressionMatch.hpp"
#include "matches/RegexMatch.hpp"
#include "matches/StringMatch.hpp"

char const* H2O::g_currentFile;
char const* H2O::g_currentSource;

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

bool declare_param(BytecodeWriter& writer, ExpressionMatch const& match)
{
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

bool divide(BytecodeWriter& writer, ExpressionMatch const& match)
{
	if(!match["LHS"][0][0]->Translate(writer))
	{
		return false;
	}
	if(!match["RHS"][0][0]->Translate(writer))
	{
		return false;
	}
	writer.Function_Call("div");
	return true;
}

bool sub(BytecodeWriter& writer, ExpressionMatch const& match)
{
	if(!match["LHS"][0][0]->Translate(writer))
	{
		return false;
	}
	if(!match["RHS"][0][0]->Translate(writer))
	{
		return false;
	}
	writer.Function_Call("sub");
	return true;
}

bool Function(BytecodeWriter& writer, ExpressionMatch const& match)
{
	writer.StartFunction(static_cast<RegexMatch*>(match["function_name"][0][0])->Group(0));
	if(!match["parameters"][0][0]->Translate(writer))
	{
		return false;
	}

	auto& lines = match["lines"][0];
	for(auto& line : lines)
	{
		if(!line->Translate(writer))
		{
			return false;
		}
	}

	writer.EndFunction();
	return true;
}

bool CallFunction(BytecodeWriter& writer, ExpressionMatch const& match)
{
	if(!match["parameters"][0][0]->Translate(writer))
	{
		return false;
	}
	writer.Function_Call(static_cast<RegexMatch*>(match["function_name"][0][0])->Group(0));
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

bool H2O::CompileFile(char const* filename, char const* definitionFile)
{
	H2OCompiler compiler;
	compiler.AddExpressionTranslator("Mult", mult);
	compiler.AddExpressionTranslator("Add", add);
	compiler.AddExpressionTranslator("Div", divide);
	compiler.AddExpressionTranslator("Sub", sub);
	compiler.AddExpressionTranslator("DeclareVariable", declare_var);
	compiler.AddExpressionTranslator("DeclareParameter", declare_param);
	compiler.AddExpressionTranslator("Function", Function);
	compiler.AddExpressionTranslator("CallFunction", CallFunction);

	compiler.AddSpanTranslator("ReadString", ReadString);

	compiler.AddLiteralTranslator("ReadDecInt", ReadDecInt);
	compiler.AddLiteralTranslator("ReadDecFloat", ReadDecFloat);

	FILE* f = fopen(definitionFile, "r");

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* fileBuffer = new char[fileSize];
	fread(fileBuffer, 1, fileSize, f);

	g_currentSource = fileBuffer;
	g_currentFile = "test.h2o";

	if(!ProcessH2OFile(fileBuffer, fileSize, compiler))
	{
		fprintf(stderr, "\nInvalid definition file.\n");
		return false;
	}

	FILE* osFile = fopen(filename, "r");
	if(!osFile)
	{
		puts("Cannot open file.");
		return false;
	}

	fseek(osFile, 0, SEEK_END);
	long osFileSize = ftell(osFile);
	fseek(osFile, 0, SEEK_SET);

	char* osFileBuffer = new char[osFileSize];
	fread(osFileBuffer, 1, osFileSize, osFile);

	g_currentSource = osFileBuffer;
	g_currentFile = filename;

	TokenList tokens;
	if(compiler.Tokenize(sprawl::StringLiteral(osFileBuffer, osFileSize), tokens))
	{
		sprawl::String result;
		if(compiler.Parse(tokens, result))
		{
			char outName[256];
			sprintf(outName, "%s.occ", filename);
			FILE* f = fopen(outName, "w");
			fwrite(result.c_str(), sizeof(char), result.length(), f);
			fclose(f);
			return true;
		}
	}

	return false;
}
