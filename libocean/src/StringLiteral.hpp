#pragma once

#include <string_view>

std::string_view StringLiteral(const char(&ptr)[N])
{
	return std::string_view(ptr, N - 1);
}