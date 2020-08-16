#include "Logging.hpp"
#include <string_view>
#include "TokenList.hpp"
#include "h2o.hpp"

namespace LoggingStatic
{
	static void PrintSource(char const* sourceCode, int line, int startColumn, int endLine, int endColumn, FILE* stream)
	{
		int currentLine = 0;
		if(line != 0)
		{
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
				++currentLine;
				if(currentLine >= endLine + 1)
				{
					break;
				}
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
			if(column != endColumn - 1 && lineStart[column+1] == '\t')
			{
				putc('\t', stream);
			}
			else
			{
				putc('~', stream);
			}
		}
		putc('\n', stream);
	}
}

void LogError(std::string const& str, TokenList const& problematicTokens)
{
	int line = -1;
	int endLine = 0;
	int startColumn = -1;
	int endColumn = 0;
	if(problematicTokens.Length() != 0)
	{
		line = problematicTokens[0].LineNo();
		startColumn = problematicTokens[0].Column();
		endColumn = 0;
		int idx = 0;
		// Currently only showing the first line, but leaving interfaces in place to allow showing multiple lines later if that proves valuable.
		// Remove the check for LineNo() == line here to activate that interface.
		while(idx < problematicTokens.Length() && problematicTokens[idx].LineNo() == line)
		{
			endColumn = problematicTokens[idx].Column() + problematicTokens[idx].Text().length() - 1;
			endLine = problematicTokens[idx].LineNo();
			++idx;
		}
	}
	if(line != -1)
	{
		fprintf(stderr, "ocean h2o: error: %s:%d:%d: %.*s\n", H2O::g_currentFile, line + 1, startColumn + 1, (int)str.length(), str.c_str());
	}
	else
	{
		fprintf(stderr, "ocean h2o: error: %s: %.*s\n", H2O::g_currentFile, (int)str.length(), str.c_str());
	}
	LoggingStatic::PrintSource(H2O::g_currentSource, line, startColumn, endLine, endColumn, stderr);
}



void LogWarning(std::string const& str, TokenList const& problematicTokens)
{
	int line = -1;
	int endLine = 0;
	int startColumn = -1;
	int endColumn = 0;
	if(problematicTokens.Length() != 0)
	{
		line = problematicTokens[0].LineNo();
		startColumn = problematicTokens[0].Column();
		endColumn = 0;
		int idx = 0;
		// Currently only showing the first line, but leaving interfaces in place to allow showing multiple lines later if that proves valuable.
		// Remove the check for LineNo() == line here to activate that interface.
		while(idx < problematicTokens.Length() && problematicTokens[idx].LineNo() == line)
		{
			endColumn = problematicTokens[idx].Column() + problematicTokens[idx].Text().length() - 1;
			endLine = problematicTokens[idx].LineNo();
			++idx;
		}
	}
	if(line != -1)
	{
		fprintf(stderr, "ocean h2o: warning: %s:%d:%d: %.*s", H2O::g_currentFile, line, startColumn, (int)str.length(), str.c_str());
	}
	else
	{
		fprintf(stderr, "ocean h2o: warning: %s: %.*s", H2O::g_currentFile, (int)str.length(), str.c_str());
	}
	LoggingStatic::PrintSource(H2O::g_currentSource, line, startColumn, endLine, endColumn, stderr);
}
