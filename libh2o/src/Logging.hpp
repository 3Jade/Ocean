#pragma once

#include <string>

class TokenList;

void LogError(std::string const& str, TokenList const& problematicTokens);
void LogWarning(std::string const& str, TokenList const& problematicTokens);
