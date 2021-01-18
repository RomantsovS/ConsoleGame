#pragma hdrstop
#include "../precompiled.h"

bool AssertFailed(const std::string& file, int line, const std::string& expression)
{
	common->Warning("ASSERTION FAILED! %s(%d): '%s'", file, line, expression);

	return true;
}
