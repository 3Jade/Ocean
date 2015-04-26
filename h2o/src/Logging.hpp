#pragma once

namespace sprawl
{
	class String;
}
class TokenList;

void LogError(sprawl::String const& str, TokenList const& problematicTokens);
void LogWarning(sprawl::String const& str, TokenList const& problematicTokens);
