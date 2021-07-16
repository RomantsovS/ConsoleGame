#pragma hdrstop
#include <precompiled.h>

struct skippedAssertion_t {
	skippedAssertion_t() {
	}

	std::string file = "";
	int				line = -1;
};
static std::vector<skippedAssertion_t> skippedAssertions;

bool AssertFailed(const std::string& file, int line, const std::string& expression) {
	// Set this to true to skip ALL assertions, including ones YOU CAUSE!
	static volatile bool skipAllAssertions = false;
	if (skipAllAssertions) {
		return false;
	}

	// Set this to true to skip ONLY this assertion
	static volatile bool skipThisAssertion = false;
	skipThisAssertion = false;

	for (size_t i = 0; i < skippedAssertions.size(); i++) {
		if (skippedAssertions[i].file == file && skippedAssertions[i].line == line) {
			skipThisAssertion = true;
			// Set breakpoint here to re-enable
			if (!skipThisAssertion) {
			}
			return false;
		}
	}

	idLib::Warning("ASSERTION FAILED! %s(%d): '%s'", file.c_str(), line, expression.c_str());

	if (IsDebuggerPresent()) {
		__debugbreak();
	}

	if (skipThisAssertion) {
		skippedAssertion_t skipped;
		skipped.file = file;
		skipped.line = line;

		skippedAssertions.push_back(skipped);
	}

	return true;
}
