#pragma once

namespace H2O
{
	extern char const* g_currentFile;
	extern char const* g_currentSource;

	bool CompileFile(char const* filename, char const* definitionFile);
}
