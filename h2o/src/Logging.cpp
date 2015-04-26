#include "Logging.hpp"
#include <sprawl/string/String.hpp>
#include "TokenList.hpp"
#include "h2o.hpp"

namespace LoggingStatic
{
	static void PrintSource(char const* sourceCode, int line, int startColumn, int endColumn, FILE* stream)
	{
		int currentLine = 0;
		while(*sourceCode != '\0')
		{
			if(*sourceCode == '\n')
			{
				++currentLine;
				if(currentLine == line)
				{
					++sourceCode;
					break;
				}
			}
			++sourceCode;
		}
		if(currentLine != line)
		{
			return;
		}
		char const* lineStart = sourceCode;
		while(*sourceCode != '\0')
		{
			putc(*sourceCode, stream);
			++sourceCode;
			if(*sourceCode == '\n')
			{
				break;
			}
		}
		putc('\n', stream);
		int column = 0;
		for(; column < startColumn; ++column)
		{
			if(lineStart[column] == '\0')
			{
				putc('^', stream);
				return;
			}
			if(lineStart[column] == '\t')
			{
				putc('\t', stream);
			}
			else
			{
				putc(' ', stream);
			}
		}
		putc('^', stream);
		for(; column < endColumn; ++column)
		{
			putc('~', stream);
		}
		putc('\n', stream);
	}
}

void LogError(sprawl::String const& str, TokenList const& problematicTokens)
{
	int line = -1;
	int startColumn = -1;
	int endColumn = 0;
	if(problematicTokens.Length() != 0)
	{
		line = problematicTokens[0].LineNo();
		startColumn = problematicTokens[0].Column();
		endColumn = 0;
		if(problematicTokens[problematicTokens.Length() - 1].LineNo() == line)
		{
			endColumn = problematicTokens[problematicTokens.Length() - 1].Column() + problematicTokens[problematicTokens.Length() - 1].Text().GetLength() - 1;
		}
	}
	if(line != -1)
	{
		fprintf(stderr, "error: %s:%d:%d: %.*s\n", g_currentFile, line + 1, startColumn + 1, (int)str.length(), str.c_str());
	}
	else
	{
		fprintf(stderr, "error: %s: %.*s\n", g_currentFile, (int)str.length(), str.c_str());
	}
	LoggingStatic::PrintSource(g_currentSource, line, startColumn, endColumn, stderr);
}



void LogWarning(sprawl::String const& str, TokenList const& problematicTokens)
{
	int line = -1;
	int startColumn = -1;
	int endColumn = 0;
	if(problematicTokens.Length() != 0)
	{
		line = problematicTokens[0].LineNo();
		startColumn = problematicTokens[0].Column();
		endColumn = 0;
		if(problematicTokens[problematicTokens.Length() - 1].LineNo() == line)
		{
			endColumn = problematicTokens[problematicTokens.Length() - 1].Column();
		}
	}
	if(line != -1)
	{
		fprintf(stderr, "warning: %s:%d:%d: %.*s", g_currentFile, line, startColumn, (int)str.length(), str.c_str());
	}
	else
	{
		fprintf(stderr, "warning: %s: %.*s", g_currentFile, (int)str.length(), str.c_str());
	}
	LoggingStatic::PrintSource(g_currentSource, line, startColumn, endColumn, stderr);
}
