#include "sys_assert.h"
#include "../../framework/Common_local.h"

bool AssertFailed(const std::string& file, int line, const std::string& expression)
{
	common->Warning("ASSERTION FAILED! %s(%d): '%s'", file, line, expression);

	return true;
}
